#include "stdafx.h"
#include "Collision.h"

#include "RigidBody.h"
#include "Reflector.h"

namespace Collision {

	namespace detail {
		////////////////////////////////////////////////////////////////////////
		// CLOSEST POINT FUNCTIONS
		////////////////////////////////////////////////////////////////////////

		struct ClosestPointsQuery {
			Vec3 pt_a = Vec3(NAN);							// pt on shape a in world space
			Vec3 pt_b = Vec3(NAN);							// pt on shape b in world space
			Float32 d = std::numeric_limits<Float32>::max();// sqr distance
		};

		static inline Float32 SignedDist(Vec3 const& pt, Plane const& p) {
			return glm::dot(pt, p.normal) - p.d;
		}

		static Vec3 ClosestPointWorld(Vec3 const& pt, Plane const& p) {
			return pt - p.normal * SignedDist(pt, p);
		}

		static Vec3 ClosestPointWorld(Vec3 const& pt, AABB const& b) {
			return glm::clamp(pt, b.position - b.halfwidths, b.position + b.halfwidths);
		}

		static Vec3 ClosestPointWorld(Vec3 const& pt, Sphere const& s) {
			Vec3 const s_position = s.GetWorldPosition();
			Vec3 const d = pt - s_position;
			if (glm::length2(d) > 0.0001f) {
				return glm::normalize(d) * s.GetRadius() + s_position;
			}

			// If pt is at the center of the sphere
			return s_position + Vec3(s.GetRadius(), 0, 0);
		}

		static Vec3 ClosestPointWorld(Vec3 const& pt, LineSegment const& ls) {
			Vec3 const s = ls.end - ls.start;
			SIK_ASSERT(glm::length2(s) > 0.0001f, "Line segment is poorly defined. Start == End.");

			// Project pt onto ls, computing parameterized position d(t) = start + t*(end - start)
			Float32 t = glm::dot(pt - ls.start, s) / glm::length2(s);

			// If outside segment, clamp t (and therefore d) to the closest endpoint
			if (t < 0.0f) { t = 0.0f; }
			if (t > 1.0f) { t = 1.0f; }

			// Compute projected position from the clamped t
			return ls.start + t * s;
		}

		static ClosestPointsQuery ClosestPoints(Sphere const& a, Sphere const& b) {
			ClosestPointsQuery result{};
			result.pt_a = ClosestPointWorld(b.GetWorldPosition(), a);
			result.pt_b = ClosestPointWorld(a.GetWorldPosition(), b);
			result.d = glm::distance2(result.pt_a, result.pt_b);
			return result;
		}

		static ClosestPointsQuery ClosestPoints(Sphere const& a, Capsule const& b) {
			ClosestPointsQuery result{};
			Vec3 const L = ClosestPointWorld(a.GetWorldPosition(), b.GetCentralSegment()); // closest point on segment of b

			Sphere s{ b.GetRadius() };
			s.SetRelativePosition(L);

			return ClosestPoints(a, s);
		}
		;
		// See Ericson Realtime Collision Detection Ch 5.1.9
		static ClosestPointsQuery ClosestPoints(LineSegment const& a, LineSegment const& b) {
			Vec3 const d1 = a.end - a.start; // Direction vector of segment a
			Vec3 const d2 = b.end - b.start; // Direction vector of segment b
			Vec3 const r = a.start - b.start;
			Float32 const La = glm::length2(d1);  // Squared length of segment a, always nonnegative
			Float32 const Lb = glm::length2(d2);  // Squared length of segment b, always nonnegative
			Float32 const f = glm::dot(d2, r);

			Float32 t{}, s{};

			// Check if either or both segments degenerate into points
			if (La <= 0.0001f && Lb <= 0.0001f) {
				// Both segments degenerate into points
				return ClosestPointsQuery{ a.start, b.start, glm::distance2(a.start, b.start) };
			}
			if (La <= 0.0001f) {
				// First segment degenerates into a point
				t = f / Lb;
				t = glm::clamp(t, 0.0f, 1.0f);
			}
			else {
				Float32 c = glm::dot(d1, r);
				if (Lb <= 0.0001f) {
					// Second segment degenerates into a point
					t = 0.0f;
					s = glm::clamp(-c / La, 0.0f, 1.0f);
				}
				else {
					// The general nondegenerate case starts here
					Float32 d = glm::dot(d1, d2);
					Float32 denom = La * Lb - d * d; // Always nonnegative

					if (denom != 0.0f) {
						s = glm::clamp((d * f - c * Lb) / denom, 0.0f, 1.0f);
					}
					else s = 0.0f;
					t = (d * s + f) / Lb;

					if (t < 0.0f) {
						t = 0.0f;
						s = glm::clamp(-c / La, 0.0f, 1.0f);
					}
					else if (t > 1.0f) {
						t = 1.0f;
						s = glm::clamp((d - c) / La, 0.0f, 1.0f);
					}
				}
			}
			Vec3 const c1 = a.start + d1 * s;
			Vec3 const c2 = b.start + d2 * t;
			return ClosestPointsQuery{ c1, c2, glm::distance2(c1, c2) };
		}

		static ClosestPointsQuery ClosestPoints(Capsule const& a, Capsule const& b) {
			ClosestPointsQuery result{};
			auto [La, Lb, d2] = ClosestPoints(a.GetCentralSegment(), b.GetCentralSegment());

			Sphere sa{ a.GetRadius() }, sb{ b.GetRadius() };
			sa.SetRelativePosition(La); // owner is world
			sb.SetRelativePosition(Lb); // owner is world

			return ClosestPoints(sa, sb);
		}

		enum class Side {
			On,
			Front,
			Back
		};

		static Side Classify(Plane const& pl, Vec3 const& pt)
		{
			static constexpr Float32 thickness = 0.05f;

			Float32 const dist = detail::SignedDist(pt, pl);

			if (dist > thickness) { return Side::Front; }
			if (dist < -thickness) { return Side::Back; }
			return Side::On;
		}

		// See Ericson 5.3.1
		static Bool Intersect(LineSegment const& seg, Plane const& plane, float& t, Vec3& q)
		{
			// Compute the t value for the directed line v intersecting the plane
			Vec3 const v = seg.end - seg.start;
			t = -SignedDist(seg.start, plane) / glm::dot(plane.normal, v);

			// If t in [0..1] compute and return intersection point
			if (t >= 0.0f && t <= 1.0f)
			{
				q = seg.start + t * v;
				return true;
			}

			// Else no intersection
			return false;
		}

		// See Ericson 8.3.4 (Sutherland-Hodgman clipping with fat planes)
		static Bool Split(Vector<Vec3> const& verts, Plane const& plane, Vector<Vec3>& front, Vector<Vec3>& back)
		{
			if (verts.empty()) { return false; }

			// Test all edges (a, b) starting with edge from last to first vertex
			auto const numVerts = verts.size();

			Vec3 a = verts.back();
			auto aSide = Classify(plane, a);

			// Loop over all edges given by vertex pair (n-1, n)
			for (auto n = 0ull; n < numVerts; n++)
			{
				Vec3 const b = verts[n];
				auto const bSide = Classify(plane, b);

				if (bSide == Side::Front)
				{
					if (aSide == Side::Back)
					{
						// Edge (a, b) straddles, output intersection point to both sides
						// Consistently clip edge as ordered going from in front -> back
						Vec3 i{};
						Float32 t{};
						Intersect(LineSegment{ .start = b, .end = a }, plane, t, i);

						SIK_ASSERT(Classify(plane, i) == Side::On, "Intersection point must be on plane");

						front.push_back(i);
						back.push_back(i);
					}
					// In all three cases, output b to the front side
					front.push_back(b);
				}
				else if (bSide == Side::Back)
				{
					if (aSide == Side::Front)
					{
						// Edge (a, b) straddles plane, output intersection point
						Vec3 i{};
						Float32 t{};
						Intersect(LineSegment{ .start = a, .end = b }, plane, t, i);

						SIK_ASSERT(Classify(plane, i) == Side::On, "Intersection point must be on plane");

						front.push_back(i);
						back.push_back(i);
					}
					else if (aSide == Side::On)
					{
						// Output a when edge (a, b) goes from ‘on’ to ‘behind’ plane
						back.push_back(a);
					}
					// In all three cases, output b to the back side
					back.push_back(b);
				}
				else
				{
					// b is on the plane. In all three cases output b to the front side
					front.push_back(b);

					// In one case, also output b to back side
					if (aSide == Side::Back)
					{
						back.push_back(b);
					}
				}

				// Keep b as the starting point of the next edge
				a = b;
				aSide = bSide;
			}

			return true;
		}

		// See Ericson 8.3.4 (Sutherland-Hodgman clipping with fat planes)
		// Note: this is currently specialized for clipping a quad against
		// another quad
		static Bool Split(Array<Vec3, 8> const& poly, Int32 numVerts, Plane const& plane,
			Array<Vec3, 8>& front, Int32& numFront, Array<Vec3, 8>& back, Int32& numBack)
		{
			if (numVerts <= 0 || numVerts > 7) { return false; }
			numFront = 0; numBack = 0; // overwrite the output arrays

			// Test all edges (a, b) starting with edge from last to first vertex
			Vec3 a = poly[numVerts - 1];
			auto aSide = Classify(plane, a);

			// Loop over all edges given by vertex pair (n-1, n)
			for (auto n = 0ull; n < numVerts; n++)
			{
				Vec3 const b = poly[n];
				auto const bSide = Classify(plane, b);

				if (bSide == Side::Front)
				{
					if (aSide == Side::Back)
					{
						// Edge (a, b) straddles, output intersection point to both sides
						// Consistently clip edge as ordered going from in front -> back
						Vec3 i{};
						Float32 t{};
						Intersect(LineSegment{ .start = b, .end = a }, plane, t, i);

						SIK_ASSERT(Classify(plane, i) == Side::On, "Intersection point must be on plane");

						front[numFront++] = i;
						back[numBack++] = i;
					}
					// In all three cases, output b to the front side
					front[numFront++] = b;
				}
				else if (bSide == Side::Back)
				{
					if (aSide == Side::Front)
					{
						// Edge (a, b) straddles plane, output intersection point
						Vec3 i{};
						Float32 t{};
						Intersect(LineSegment{ .start = a, .end = b }, plane, t, i);

						SIK_ASSERT(Classify(plane, i) == Side::On, "Intersection point must be on plane");

						front[numFront++] = i;
						back[numBack++] = i;
					}
					else if (aSide == Side::On)
					{
						// Output a when edge (a, b) goes from ‘on’ to ‘behind’ plane
						back[numBack++] = a;
					}
					// In all three cases, output b to the back side
					back[numBack++] = b;
				}
				else
				{
					// b is on the plane. In all three cases output b to the front side
					front[numFront++] = b;

					// In one case, also output b to back side
					if (aSide == Side::Back)
					{
						back[numBack++] = b;
					}
				}

				// Keep b as the starting point of the next edge
				a = b;
				aSide = bSide;
			}

			return true;
		}
	}

	// AABB methods

	Array<Vec3, 4> AABB::FaceAsPolygon(Face face) const
	{
		Float32 const sign = (static_cast<Int32>(face) >= 3) ? -1.0f : 1.0f;
		Int32 const axis = static_cast<Int32>(face) % 3;
		Int32 const j = (axis + 1) % 3;
		Int32 const k = (j + 1) % 3;

		Array<Vec3, 4> result{	
			halfwidths,
			halfwidths,
			halfwidths,
			halfwidths
		};
		// Wind around verts counter clockwise
		// (j, k) = (+, +) --> (-, +) --> (-, -) --> (+, -)
		result[1][j] *= -1.0f;
		result[2][j] *= -1.0f;
		result[2][k] *= -1.0f;
		result[3][k] *= -1.0f;

		if (sign < 0.0f) {
			for (auto&& v : result) {
				v[axis] *= -1.0f;
			}
			std::swap(result[1], result[3]); // swap to maintain counterclockwise order
		}

		return result;
	}

	LineSegment AABB::EdgeAsSegment(Face adjFace1, Face adjFace2) const
	{
		Int32 const faceIdx1 = static_cast<Int32>(adjFace1);
		Int32 const faceIdx2 = static_cast<Int32>(adjFace2);

		// Axes orthogonal to edge
		Int32   const axis1 = faceIdx1 % 3;
		Int32   const axis2 = faceIdx2 % 3;

		Float32 const sign1 = (faceIdx1 >= 3) ? -1.0f : 1.0f;
		Float32 const sign2 = (faceIdx2 >= 3) ? -1.0f : 1.0f;

		SIK_ASSERT(axis1 != axis2, "Faces are parallel, not adjacent");

		// Axis parallel to edge
		Int32 axis3 = 0;
		switch (axis1 + axis2)
		{
			break; case 2: { axis3 = 1; }
			break; case 1: { axis3 = 2; }
		}

		Vec3 e{ halfwidths };
		e[axis1] *= sign1;
		e[axis2] *= sign2;

		Vec3 b{ e };
		b[axis3] *= -1.0f;

		return LineSegment{
			.start = b,
			.end = e
		};
	}

	Vec3 AABB::FaceNormal(Face face) const
	{
		Int32 const faceIdx = static_cast<Int32>(face);
		Float32 const sign = (faceIdx >= 3) ? -1.0f : 1.0f;

		Vec3 result{ 0.0f };
		result[faceIdx % 3] = sign;

		return result;
	}

	Vec3 AABB::SupportingVertex(Vec3 const& localDir) const
	{
		Float32 bestProj = std::numeric_limits<Float32>::lowest();

		Vec3 const verts[] = {
			halfwidths,
			Vec3(-halfwidths.x, halfwidths.y, halfwidths.z),
			Vec3(halfwidths.x, -halfwidths.y, halfwidths.z),
			Vec3(halfwidths.x, halfwidths.y, -halfwidths.z),
			-halfwidths,
			Vec3(halfwidths.x, -halfwidths.y, -halfwidths.z),
			Vec3(-halfwidths.x, halfwidths.y, -halfwidths.z),
			Vec3(-halfwidths.x, -halfwidths.y, halfwidths.z)
			
		};

		Float32 proj = 0.0f;
		Int32 best = -1;

		for (Int32 i = 0; i < 8; ++i) {
			proj = glm::dot(verts[i], localDir);
			if (proj > bestProj) {
				bestProj = proj;
				best = i;
			}
		}

		return verts[best];
	}

	AABB::Face AABB::SupportingFace(Vec3 const& localDir) const
	{
		static constexpr Int32 numFaces = static_cast<Int32>(Face::COUNT); // 6 

		Face best{};
		Float32 bestProj = std::numeric_limits<Float32>::lowest();
		for (Int32 i = 0; i < numFaces; ++i)
		{
			Face const f = static_cast<Face>(i);
			Float32 const dot = glm::dot(FaceNormal(f), localDir);
			if (dot > bestProj)
			{
				bestProj = dot;
				best = f;
			}
		}
		return best;
	}


	LineSegment AABB::SupportingEdgeWithDirection(Int32 axisIdx, Vec3 const& localDir) const
	{
		Vec3 const support = SupportingVertex(localDir);
		Vec3 other = support;
		other[axisIdx] *= -1.0f;
		return LineSegment{ .start = other, .end = support };

		//// We test two adjacent vertices on the face given by axisIdx, starting 
		//// with extents
		//Vec3 verts[2] = {
		//	halfwidths,
		//	halfwidths
		//};

		//// For the second vert, we need to do some index trickery
		//Int32 const j = (axisIdx + 1) % 3;
		//verts[1][j] *= -1.0f;

		//// Choose the vert with the best (abs) projection onto dir
		//Int32 vIdx = 0;
		//Float32 proj = glm::dot(verts[0], localDir);
		//if (Float32 const proj1 = glm::dot(verts[1], localDir);
		//	glm::abs(proj1) > glm::abs(proj))
		//{
		//	proj = proj1;
		//	vIdx = 1;
		//}

		//// If the projection was negative, flip to the opposite vertex 
		//// on the same face (e.g. diagonal from this vert)
		//if (proj < 0.0f)
		//{
		//	Int32 const k = (j + 1) % 3;
		//	verts[vIdx][j] *= -1.0f;
		//	verts[vIdx][k] *= -1.0f;
		//}

		//// Get the adjacent vertex on the opposite face in axisIdx direction
		//LineSegment result
		//{
		//	.start = verts[vIdx],
		//	.end = verts[vIdx]
		//};
		//result.end[axisIdx] *= -1.0f;
		//
		//return result;
	}

	LineSegment AABB::SupportingEdge(Vec3 const& localDir) const
	{
		// Identify the most orthogonal axis to localDir
		Int32 axisIdx = 0;
		{
			Float32 bestProj = glm::abs(localDir[0]);
			Float32 proj{};
			for (Int32 i = 1; i < 2; ++i)
			{
				proj = glm::abs(localDir[i]);
				if (proj < bestProj)
				{
					bestProj = proj;
					axisIdx = i;
				}
			}
		}

		return SupportingEdgeWithDirection(axisIdx, localDir);
	}

	ContactManifold AABB::Collide(AABB const& other) const {
		static constexpr Float32 y_bias = 0.01f;

		// Objects are spawned on top of each other
		if (glm::all(glm::epsilonEqual(position, other.position, 0.001f))) {
			return ContactManifold{ .normal = Vec3(1,0,0) };
		}

		// Find penetration on all axes
		Vec3 pen = (halfwidths + other.halfwidths) - glm::abs(position - other.position);
		pen.y = std::max(pen.y - y_bias, 0.0f); // prioritize y-axis since ground collisions are most common

		//// If there is no penetration on any axis, we have no contacts
		//if (glm::any(glm::lessThan(pen, Vec3(0.0f)))) { return ContactManifold{}; }

		// Find the least penetration
		Float32 const min_pen = std::min({ pen.x, pen.y, pen.z });

		// Find axis of least penetration and contact point (could find 4, but one will do since AABBs don't rotate)
		Vec3 contact{};
		Vec3 normal{};
		if (min_pen == pen.y) {
			Float32 const sign = position.y < other.position.y ? 1.0f : -1.0f;
			normal = sign * Vec3(0, 1, 0);
			contact = Vec3{
				std::min(position.x + halfwidths.x, other.position.x + other.halfwidths.x),
				sign * (halfwidths.y - 0.5f * min_pen) + position.y,
				std::min(position.z + halfwidths.z, other.position.z + other.halfwidths.z)
			};
		}
		else if (min_pen == pen.x) {
			Float32 const sign = position.x < other.position.x ? 1.0f : -1.0f;
			normal = sign * Vec3(1, 0, 0);
			contact = Vec3{
				sign * (halfwidths.x - 0.5f * min_pen) + position.x,
				std::min(position.y + halfwidths.y, other.position.y + other.halfwidths.y),
				std::min(position.z + halfwidths.z, other.position.z + other.halfwidths.z),
			};
		}
		else {
			Float32 const sign = position.z < other.position.z ? 1.0f : -1.0f;
			normal = sign * Vec3(0, 0, 1);
			contact = Vec3{
				std::min(position.x + halfwidths.x, other.position.x + other.halfwidths.x),
				std::min(position.y + halfwidths.y, other.position.y + other.halfwidths.y),
				sign * (halfwidths.z - 0.5f * min_pen) + position.z
			};
		}

		ContactManifold result{ .normal = normal };

		if (min_pen >= 0.0f) {
			result.contacts[result.num_contacts++] = {
				.position = contact,
				.penetration = min_pen
			};
		}

		return result;
	}

	// Specialized SAT for OBB: See Ericson 4.4.1-2
	ContactManifold AABB::CollideAsOBB(Quat const& oA, AABB const& B, Quat const& oB) const
	{
		// These epsilon values are used to prevent issues with parallel axes
		static constexpr Float32 epsilon = 1.0e-4f;
		static constexpr Mat3 epsilonMat3 = Mat3{ Vec3(epsilon), Vec3(epsilon), Vec3(epsilon) };

		// These values are used to make Face A preferable to Face B, and both faces
		// preferable to an Edge Pair when deciding what contact to generate.
		// Since values are negative, this multiplier makes values closer
		// to zero, therefore more preferable.
		static constexpr Float32 relEdgeTolerance = 0.90f;
		static constexpr Float32 relFaceTolerance = 0.99f;
		static constexpr Float32 absTolerance = epsilon;

		// Perform computations in local space of A
		Mat4 const trA = glm::translate(Mat4(1), position) * glm::toMat4(oA);
		Mat4 const trB = glm::translate(Mat4(1), B.position) * glm::toMat4(oB);
		Quat const invOA = glm::inverse(oA);
		Mat3 const R = glm::toMat3( invOA * oB );  // expresses B in coordinate frame of A
		Mat3 const absR = Mat3(glm::abs(R[0]), glm::abs(R[1]), glm::abs(R[2]));
		Vec3 const worldT = B.position - position;
		Vec3 const t = glm::rotate(invOA, worldT);

		// Helper to go from an axis index to a vector in local space of A
		auto IndexToAxis = [&](Int32 axisIndex) -> Vec3 {
			switch (axisIndex) {
			break; case 0: { return Vec3(1, 0, 0); }
			break; case 1: { return Vec3(0, 1, 0); }
			break; case 2: { return Vec3(0, 0, 1); }
			break; case 3: { return R * Vec3(1, 0, 0); }
			break; case 4: { return R * Vec3(0, 1, 0); }
			break; case 5: { return R * Vec3(0, 0, 1); }

			break; case 6: { return glm::normalize(glm::cross(Vec3(1, 0, 0), R * Vec3(1, 0, 0))); }
			break; case 7: { return glm::normalize(glm::cross(Vec3(1, 0, 0), R * Vec3(0, 1, 0))); }
			break; case 8: { return glm::normalize(glm::cross(Vec3(1, 0, 0), R * Vec3(0, 0, 1))); }
			break; case 9: { return glm::normalize(glm::cross(Vec3(0, 1, 0), R * Vec3(1, 0, 0))); }
			break; case 10: { return glm::normalize(glm::cross(Vec3(0, 1, 0), R * Vec3(0, 1, 0))); }
			break; case 11: { return glm::normalize(glm::cross(Vec3(0, 1, 0), R * Vec3(0, 0, 1))); }
			break; case 12: { return glm::normalize(glm::cross(Vec3(0, 0, 1), R * Vec3(1, 0, 0))); }
			break; case 13: { return glm::normalize(glm::cross(Vec3(0, 0, 1), R * Vec3(0, 1, 0))); }
			break; case 14: { return glm::normalize(glm::cross(Vec3(0, 0, 1), R * Vec3(0, 0, 1))); }
			}
			return Vec3(0);
		};

		// Helper to obtain clipped incident face in the local space of the reference box
		auto ClipIncidentToReference = [](AABB const& ref, AABB const& inc, Int32 refAxis, AABB::Face incFace, Mat4 const& incToRef) -> std::pair<Array<Vec3, 8>, Int32>
		{
			Array<Vec3, 4> poly = inc.FaceAsPolygon(incFace);
			for (auto&& v : poly)
			{
				v = incToRef * Vec4(v, 1);
			}
			Array<Vec3, 8> result{};
			Array<Vec3, 8> front{}, back{};
			Int32 numVerts = 4, numFront = 0, numBack = 0;
			std::copy(poly.begin(), poly.end(), result.begin());

			Int32 const j = (refAxis + 1) % 3;
			Int32 const k = (j + 1) % 3;
			Plane clipPlane{ .normal = Vec3(0), .d = 0 };

			{
				clipPlane.normal[j] = 1.0f;
				clipPlane.d = ref.halfwidths[j];

				if (not detail::Split(result, numVerts, clipPlane, front, numFront, back, numBack)) { return { result, numVerts }; }
				std::copy(back.begin(), back.begin() + numBack, result.begin());
				numVerts = numBack;

				clipPlane.normal[j] = 0.0f;
			}
			{
				clipPlane.normal[k] = 1.0f;
				clipPlane.d = ref.halfwidths[k];

				if (not detail::Split(result, numVerts, clipPlane, front, numFront, back, numBack)) { return { result, numVerts }; }
				std::copy(back.begin(), back.begin() + numBack, result.begin());
				numVerts = numBack;

				clipPlane.normal[k] = 0.0f;
			}
			{
				clipPlane.normal[j] = -1.0f;
				clipPlane.d = ref.halfwidths[j];

				if (not detail::Split(result, numVerts, clipPlane, front, numFront, back, numBack)) { return{ result, numVerts }; }
				std::copy(back.begin(), back.begin() + numBack, result.begin());
				numVerts = numBack;

				clipPlane.normal[j] = 0.0f;
			}
			{
				clipPlane.normal[k] = -1.0f;
				clipPlane.d = ref.halfwidths[k];

				if (not detail::Split(result, numVerts, clipPlane, front, numFront, back, numBack)) { return { result, numVerts }; }
				std::copy(back.begin(), back.begin() + numBack, result.begin());
				numVerts = numBack;
			}
			return { result, numVerts };
		};


		// We keep track of the axis of greatest separation
		Int32   bestFaceAxisA = -1;
		Int32   bestFaceAxisB = -1;
		Int32   bestEdgeAxisA = -1, bestEdgeAxisB = -1;
		Float32 bestFaceSepA = std::numeric_limits<Float32>::lowest();
		Float32 bestFaceSepB = std::numeric_limits<Float32>::lowest();
		Float32 bestEdgeSep = std::numeric_limits<Float32>::lowest();

		ContactManifold result{};

		// Tests for face normals of A
		Vec3 const faceSepsA = glm::abs(t) - (halfwidths + absR * B.halfwidths);
		for (Int32 i = 0; i < 3; ++i) {
			if (faceSepsA[i] > 0.0f) {
				Vec3 localDir{ 0.0f };
				localDir[i] = 1.0f;

				result.normal = glm::rotate(oA, localDir);
				if (glm::dot(result.normal, worldT) < 0.0f) {
					result.normal *= -1.0f;
				}
				return result;
			}
			else if (faceSepsA[i] > bestFaceSepA) {
				bestFaceSepA = faceSepsA[i];
				bestFaceAxisA = i;
			}
		}

		// Tests for face normals of B
		Vec3 const faceSepsB = glm::abs(glm::transpose(R) * t) - (glm::transpose(absR) * halfwidths + B.halfwidths);
		for (Int32 i = 0; i < 3; ++i) {
			if (faceSepsB[i] > 0.0f) {
				Vec3 localDir{ 0.0f };
				localDir[i] = 1.0f;

				result.normal = glm::rotate(oB, localDir);
				if (glm::dot(result.normal, worldT) < 0.0f) {
					result.normal *= -1.0f;
				}
				return result;
			}
			else if (faceSepsB[i] > bestFaceSepB) {
				bestFaceSepB = faceSepsB[i];
				bestFaceAxisB = i;
			}
		}

		//// Tests for axisA x axisB -- since we are disallowing rotations, we may omit this
		//{
		//	Int32 m = 1, n = 2;
		//	for (Int32 a = 0; a < 3; ++a)
		//	{
		//		Vec3 localA{ 0.0f };
		//		localA[a] = 1.0f;

		//		Int32 j = 1, k = 2;

		//		for (Int32 b = 0; b < 3; ++b)
		//		{
		//			Float32 const ra = halfwidths[m] * absR[b][n] + halfwidths[n] * absR[b][m];
		//			Float32 const rb = B.halfwidths[j] * absR[k][a] + B.halfwidths[k] * absR[j][a];

		//			Float32 const sep = glm::abs(t[n] * R[b][m] - t[m] * R[b][n]) - (ra + rb);

		//			Vec3 localB{ 0.0f };
		//			localB[b] = 1.0f;

		//			if (sep > 0.0f) {
		//				result.normal = glm::cross( glm::rotate(oA, localA), glm::rotate(oB, localB) );
		//				if (glm::dot(result.normal, worldT) < 0.0f) {
		//					result.normal *= -1.0f;
		//				}
		//				return result;
		//			}
		//			else if (sep > bestEdgeSep) {

		//				// If edges are not parallel, record the axis
		//				if (glm::abs(glm::dot(R * localB, localA)) + epsilon < 1.0f) {
		//					bestEdgeSep = sep;
		//					bestEdgeAxisA = a;
		//					bestEdgeAxisB = b;
		//				}
		//			}
		//			j = k;
		//			k = b;
		//		}
		//		m = n;
		//		n = a;
		//	}
		//}


		Bool const edgeContact = bestEdgeSep > (relEdgeTolerance * glm::max(bestFaceSepA, bestFaceSepB) + absTolerance);
		Bool const faceBContact = bestFaceSepB > (relFaceTolerance * bestFaceSepA + absTolerance);

		Vec3 normalLocalA{};

		if (edgeContact) {
			// Compute contact normal in local space of A
			normalLocalA = IndexToAxis((bestEdgeAxisA * 3 + bestEdgeAxisB) + 6);
			if (glm::dot(normalLocalA, t) < 0.0f) { // flip s.t. pointing A->B
				normalLocalA *= -1.0f;
			}

			// Identify which edges (in world space) are in contact
			LineSegment const edgeA = SupportingEdgeWithDirection(bestEdgeAxisA, normalLocalA)
				.Transform(glm::toMat4(oA));
			LineSegment const edgeB = B.SupportingEdgeWithDirection(bestEdgeAxisB, glm::transpose(R) * -normalLocalA)
				.Transform(glm::toMat4(oB));

			// Compute closest points in world space
			auto const closestPts = detail::ClosestPoints(edgeA, edgeB);

			// Build a contact point at midpoint between the two closest points
			result.contacts[result.num_contacts++] = Contact{
				.position = 0.5f * (closestPts.pt_a + closestPts.pt_b),
				.penetration = -bestEdgeSep
			};
		}
		else {
			if (faceBContact) {
				// Compute normal in local space of A
				normalLocalA = IndexToAxis(bestFaceAxisB + 3);
				if (glm::dot(normalLocalA, t) < 0.0f) { // flip s.t. pointing A->B
					normalLocalA *= -1.0f;
				}
				Vec3 const normalLocalB = glm::transpose(R) * normalLocalA;

				auto const incidentFace = SupportingFace(normalLocalA);
				auto [clipFace, numClipVerts] = ClipIncidentToReference(B, *this, bestFaceAxisB, incidentFace, glm::inverse(trB) * trA);

				SIK_ASSERT(0 <= numClipVerts && numClipVerts <= 8, "Invalid contact point count");

				// Compute penetration depths and project clip verts onto ref plane
				Plane const refFacePlane{ .normal = -normalLocalB, .d = B.halfwidths[bestFaceAxisB] };
				Float32 depths[8] = {};
				for (Int32 i = 0; i < numClipVerts; ++i) {
					depths[i] = detail::SignedDist(clipFace[i], refFacePlane);
					clipFace[i] -= depths[i] * refFacePlane.normal;
				}

				// Finally, generate the contact points
				for (Int32 i = 0; i < numClipVerts; ++i) {
					if (depths[i] < epsilon) {
						result.contacts[result.num_contacts++] = Contact{
							.position = trB * Vec4( clipFace[i], 1),
							.penetration = -depths[i]
						};
					}
				}
			}
			else { // Face A is reference

				// Compute normal in local space of A, and in world space, pointing A->B
				normalLocalA = IndexToAxis(bestFaceAxisA);
				if (glm::dot(normalLocalA, t) < 0.0f) {
					normalLocalA *= -1.0f;
				}
				Vec3 const normalLocalB = glm::transpose(R) * normalLocalA;

				auto const incidentFace = B.SupportingFace(-normalLocalB);
				auto [clipFace, numClipVerts] = ClipIncidentToReference(*this, B, bestFaceAxisA, incidentFace, glm::inverse(trA) * trB);

				SIK_ASSERT(0 <= numClipVerts && numClipVerts <= 8, "Invalid contact point count");

				// Compute penetration depths and project clip verts onto ref plane
				Plane const refFacePlane{ .normal = normalLocalA, .d = halfwidths[bestFaceAxisA] };
				Float32 depths[8] = {};
				for (Int32 i = 0; i < numClipVerts; ++i) {
					depths[i] = detail::SignedDist(clipFace[i], refFacePlane);
					clipFace[i] -= depths[i] * refFacePlane.normal;
				}

				// Finally, generate the contact points
				for (Int32 i = 0; i < numClipVerts; ++i) {
					if (depths[i] < epsilon) {
						result.contacts[result.num_contacts++] = Contact{
							.position = trA * Vec4(clipFace[i], 1),
							.penetration = -depths[i]
						};
					}
				}
			}
		}

		result.normal = oA * normalLocalA;

		SIK_ASSERT(glm::epsilonEqual(glm::length2(result.normal), 1.0f, epsilon), "Invalid normal");
		return result;
	}


	// Collider methods

	Collider::Collider(Type t) 
		: owner { &RigidBody::world_body }, 
		local_to_body{ 1.0f },
		local_to_world{ 1.0f },
		inertia_local{ 1.0f },
		mass{ 0.0f },
		type{ t }
	{}

	void Collider::UpdateWorldTransform() {
		local_to_world = owner->TransformMatrix() * local_to_body;
	}

	void Collider::SetOwner(RigidBody const* o) {
		owner = o;
		UpdateWorldTransform();
	}

	// Sphere methods

	Sphere::Sphere(Float32 r)
		: Collider(Type::Sphere), radius{ r }
	{
		ComputeInertiaTensor();
	}

	AABB Sphere::GetBoundingBox() const {
		return AABB{
			.position = GetWorldPosition(),
			.halfwidths = Vec3(radius)
		};
	}


	// Capsule methods

	Capsule::Capsule()
		: Collider(Type::Capsule), radius{ 1.0f }, seg_halflength{ 1.0f }
	{
		ComputeInertiaTensor();
	}
	
	Capsule::Capsule(Float32 r, Float32 h)
		: Collider(Type::Capsule), radius{ r }, seg_halflength{ 0.5f * h }
	{
		ComputeInertiaTensor();
	}
	
	AABB Capsule::GetBoundingBox() const {
		Vec3 const bounds_local{ radius, seg_halflength + radius, radius };
		
		Mat3 const rot = GetWorldRotationMat3();
		AABB result{ .position = GetWorldPosition(), .halfwidths = Vec3(0) };

		for (auto i = 0u; i < 3u; ++i) {
			for (auto j = 0u; j < 3u; ++j) {
				result.halfwidths[i] += std::abs(rot[i][j]) * bounds_local[j];
			}
		}

		return result;
	}

	// Hull methods

	Hull::Hull()
		: Collider(Type::Hull),
		bounds{},
		vertices{},
		edges{},
		faces{},
		planes{}
	{}

	AABB Hull::GetBoundingBox() const {
		Mat3 const rot = GetWorldRotationMat3();
		AABB result{ .position = GetWorldPosition(), .halfwidths = Vec3(0) };

		for (auto i = 0u; i < 3u; ++i) {
			for (auto j = 0u; j < 3u; ++j) {
				result.halfwidths[i] += std::abs(rot[i][j]) * bounds[j];
			}
		}

		return result;
	}

	Hull Hull::BoxInstance(Vec3 const& halfwidths) {

        // Every edge is adjacent to its twin. Other info is arbitrary.
        static constexpr Array<HalfEdge, 24> edges = {
																	// Edge Index
			HalfEdge{.next = 2, .twin = 1, .origin = 0, .face = 0}, //  0
			HalfEdge{.next = 8, .twin = 0, .origin = 1, .face = 1}, //  1

			HalfEdge{.next = 4, .twin = 3, .origin = 1, .face = 0}, //  2
			HalfEdge{.next = 13, .twin = 2, .origin = 2, .face = 2}, //  3

			HalfEdge{.next = 6, .twin = 5, .origin = 2, .face = 0}, //  4
			HalfEdge{.next = 21, .twin = 4, .origin = 3, .face = 3}, //  5

			HalfEdge{.next = 0, .twin = 7, .origin = 3, .face = 0}, //  6
			HalfEdge{.next = 14, .twin = 6, .origin = 0, .face = 4}, //  7

			HalfEdge{.next = 10, .twin = 9, .origin = 0, .face = 1},  //  8
			HalfEdge{.next = 7, .twin = 8, .origin = 4, .face = 4},  //  9

			HalfEdge{.next = 12, .twin = 11, .origin = 4, .face = 1},  // 10
			HalfEdge{.next = 17, .twin = 10, .origin = 5, .face = 5},  // 11

			HalfEdge{.next = 1, .twin = 13, .origin = 5, .face = 1},  // 12
			HalfEdge{.next = 18, .twin = 12, .origin = 1, .face = 2},  // 13

			HalfEdge{.next = 16, .twin = 15, .origin = 3, .face = 4},  // 14
			HalfEdge{.next = 5, .twin = 14, .origin = 7, .face = 3},  // 15

			HalfEdge{.next = 9, .twin = 17, .origin = 7, .face = 4},  // 16
			HalfEdge{.next = 23, .twin = 16, .origin = 4, .face = 5},  // 17

			HalfEdge{.next = 20, .twin = 19, .origin = 5, .face = 2},  // 18
			HalfEdge{.next = 11, .twin = 18, .origin = 6, .face = 5},  // 19

			HalfEdge{.next = 3, .twin = 21, .origin = 6, .face = 2},  // 20
			HalfEdge{.next = 22, .twin = 20, .origin = 2, .face = 3},  // 21

			HalfEdge{.next = 15, .twin = 23, .origin = 6, .face = 3},  // 22
			HalfEdge{.next = 19, .twin = 22, .origin = 7, .face = 5}   // 23            
        };

        static constexpr Array<Face, 6> faces = {
            Face{.edge = 0 },  // Front 0
            Face{.edge = 1 },  // Bottom 1
            Face{.edge = 3 },  // Right 2
            Face{.edge = 5 },  // Top 3
            Face{.edge = 7 },  // Left 4
            Face{.edge = 11 },  // Back 5
        };

        Hull h{};
		h.bounds = halfwidths;
        h.edges = Vector<HalfEdge>(edges.begin(), edges.end());
        h.faces = Vector<Face>(faces.begin(), faces.end());

        h.planes = {
            Plane{.normal = Vec3(0,  0,  1), .d = halfwidths.z}, // Front
            Plane{.normal = Vec3(0, -1,  0), .d = halfwidths.y}, // Bottom
            Plane{.normal = Vec3(1,  0,  0), .d = halfwidths.x}, // Right
            Plane{.normal = Vec3(0,  1,  0), .d = halfwidths.y}, // Top
            Plane{.normal = Vec3(-1,  0,  0), .d = halfwidths.x}, // Left
            Plane{.normal = Vec3(0,  0, -1), .d = halfwidths.z}  // Back
        };

        h.vertices = {
            Vec3{ -halfwidths.x, -halfwidths.y,  halfwidths.z }, // 0
            Vec3{  halfwidths.x, -halfwidths.y,  halfwidths.z }, // 1
            Vec3{  halfwidths.x,  halfwidths.y,  halfwidths.z }, // 2
            Vec3{ -halfwidths.x,  halfwidths.y,  halfwidths.z }, // 3
            Vec3{ -halfwidths.x, -halfwidths.y, -halfwidths.z }, // 4
            Vec3{  halfwidths.x, -halfwidths.y, -halfwidths.z }, // 5
            Vec3{  halfwidths.x,  halfwidths.y, -halfwidths.z }, // 6
            Vec3{ -halfwidths.x,  halfwidths.y, -halfwidths.z }  // 7
        };

        Float32 const x2 = halfwidths.x * halfwidths.x;
        Float32 const y2 = halfwidths.y * halfwidths.y;
        Float32 const z2 = halfwidths.z * halfwidths.z;
        h.inertia_local = {
            {y2 + z2, 0,       0      },
            {0      , x2 + z2, 0      },
            {0      , 0      , x2 + y2}
        };


        return h;
	}

	void Hull::ComputeInertiaTensor() {

		Mat3 I(0);
		for (auto&& v : vertices) {
			I[0][0] += v.y * v.y + v.z * v.z;
			I[0][1] -= v.x * v.y;
			I[0][2] -= v.x * v.z;

			I[1][1] += v.x * v.x + v.z * v.z;
			I[1][2] -= v.y * v.z;

			I[2][2] += v.x * v.x + v.y * v.y;
		}

		I[1][0] = I[0][1];
		I[2][0] = I[0][2];
		I[2][1] = I[1][2];

		inertia_local = I;
	}

	////////////////////////////////////////////////////////////////////////////
	// COLLISION HELPERS
	////////////////////////////////////////////////////////////////////////////
	namespace detail {

		// SAT helpers
		struct FaceQuery {
			Int32   index = -1;
			Float32 separation = std::numeric_limits<Float32>::lowest();
			Vec3    normal = Vec3(NAN);
		};

		struct EdgeQuery {
			Int32   index1 = -1;
			Int32   index2 = -1;
			Float32 separation = std::numeric_limits<Float32>::lowest();
			Vec3	normal = Vec3(NAN);
		};

		static FaceQuery SATQueryFaceDirections(const Hull& hull1, const Hull& hull2);
		static Float32 Project(const Plane& plane, const Hull& hull);
		static EdgeQuery SATQueryEdgeDirections(const Hull& hull1, const Hull& hull2);
		static Float32 Project(const Vec3& p1, const Vec3& e1, const Vec3& p2, const Vec3& e2, const Vec3& c1, Vec3& out_normal);
		static inline Bool IsMinkowskiFace(Vec3 const& a, Vec3 const& b, Vec3 const& b_x_a, Vec3 const& c, Vec3 const& d, Vec3 const& d_x_c);
	}

	
	namespace detail {
		////////////////////////////////////////////////////////////////////////
		// INTERSECTION FUNCTIONS
		////////////////////////////////////////////////////////////////////////

		static inline Bool Intersect(AABB const& a, AABB const& b) {
			if (std::abs(a.position.x - b.position.x) > (a.halfwidths.x + b.halfwidths.x)) { return false; }
			if (std::abs(a.position.y - b.position.y) > (a.halfwidths.y + b.halfwidths.y)) { return false; }
			if (std::abs(a.position.z - b.position.z) > (a.halfwidths.z + b.halfwidths.z)) { return false; }
			return true;
		}

		static inline Bool Intersect(Sphere const& a, Sphere const& b) {
			// Sphere a, Sphere b
			return glm::distance2(a.GetWorldPosition(), b.GetWorldPosition()) <= (a.GetRadius() + b.GetRadius()) * (a.GetRadius() + b.GetRadius());
		}

		static Bool Intersect(Capsule const& a, Capsule const& b) {
			auto [pa, pb, d2] = ClosestPoints(a.GetCentralSegment(), b.GetCentralSegment());
			return d2 <= (a.GetRadius() + b.GetRadius()) * (a.GetRadius() + b.GetRadius());
		}

		static Bool Intersect(Hull const& a, Hull const& b) {
			return Intersect(a.GetBoundingBox(), b.GetBoundingBox());
		}

#define DEFN_INTERSECT_FCN(T, U) \
static Bool Intersect(T const& a, U const& b); \
static Bool Intersect(U const& a, T const& b) { return Intersect(b, a); } \
static Bool Intersect(T const& a, U const& b)

		DEFN_INTERSECT_FCN(Plane, Sphere) {
			// Plane a, Sphere b
			return SignedDist(b.GetWorldPosition(), a) <= b.GetRadius();
		}

		DEFN_INTERSECT_FCN(Plane, AABB) {
			// Plane a, AABB b
			return SignedDist(b.position, a) <= glm::dot(b.halfwidths, glm::abs(a.normal));
		}

		DEFN_INTERSECT_FCN(Sphere, AABB) {
			// Sphere a, AABB b
			Vec3 const a_position = a.GetWorldPosition();
			return glm::distance2(a_position, ClosestPointWorld(a_position, b)) <= a.GetRadius() * a.GetRadius();
		}

		DEFN_INTERSECT_FCN(Sphere, Capsule) {
			// Sphere a, Capsule b
			Vec3 const a_position = a.GetWorldPosition();
			Vec3 const L = ClosestPointWorld(a_position, b.GetCentralSegment());
			return glm::length2(a_position - L) <= glm::length2(a.GetRadius() + b.GetRadius());

		}

		DEFN_INTERSECT_FCN(Sphere, Hull) {
			// Sphere a, Hull b
			return Intersect(a, b.GetBoundingBox());
		}

		// See Ericson Realtime Collision Detection 5.3.3
		DEFN_INTERSECT_FCN(LineSegment, AABB) {
			// LineSegment a, AABB b

			Vec3 const c = b.position;
			Vec3 const e = b.halfwidths;
			Vec3 m = a.Midpoint();
			Vec3 const d = a.end - m; // Segment halflength vector
			m = m - c;                // Translate box and segment to origin

			// Try world coordinate axes as separating axes
			Float32 adx = std::abs(d.x);
			if (std::abs(m.x) > e.x + adx) { return false; }
			Float32 ady = std::abs(d.y);
			if (std::abs(m.y) > e.y + ady) { return false; }
			Float32 adz = std::abs(d.z);
			if (std::abs(m.z) > e.z + adz) { return false; }

			// Add in an epsilon term to counteract arithmetic errors when segment is
			// (near) parallel to a coordinate axis (see text for detail)
			adx += 0.0001f; ady += 0.0001f; adz += 0.0001f;

			// Try cross products of segment direction vector with coordinate axes
			if (std::abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) { return false; }
			if (std::abs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) { return false; }
			if (std::abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) { return false; }

			// No separating axis found; segment must be overlapping AABB
			return true;
		}

		DEFN_INTERSECT_FCN(Capsule, AABB) {
			// Capsule a, AABB b
			return Intersect(
				a.GetCentralSegment(),
				AABB{
					.position = b.position,
					.halfwidths = b.halfwidths + Vec3(a.GetRadius())
				});
		}
		
		DEFN_INTERSECT_FCN(Capsule, Hull) {
			// Capsule a, Hull b
			return Intersect(a, b.GetBoundingBox());
		}
		
		

#undef DEFN_INTERSECT_FCN
	}

	////////////////////////////////////////////////////////////////////////////
	// COLLISION IMPLEMENTATIONS
	////////////////////////////////////////////////////////////////////////////
	namespace detail {
		static ContactManifold Collide(Sphere const& a, Sphere const& b) {

			ContactManifold result{};

			Vec3 const a_position = a.GetWorldPosition();
			Vec3 const b_position = b.GetWorldPosition();
			Float32 const a_radius = a.GetRadius();
			Float32 const b_radius = b.GetRadius();

			Float32 const dist = glm::distance(a_position, b_position);
			if (dist < 0.0001f) {
				return result;
			}

			Float32 const pen = (a_radius + b_radius) - dist;
			if (pen > 0.0f) {

				result.normal = (b_position - a_position) / dist;

				result.contacts[result.num_contacts++] = Contact{
					// contact position is halfway between the surface points of two spheres
					.position = result.normal * (a_radius - 0.5f * pen) + a_position,
					.penetration = pen
				};
			}

			return result;
		}

		static ContactManifold Collide(Sphere const& a, Capsule const& b) {

			// Closest point to a on internal line segment of b
			Vec3 const pt_s = detail::ClosestPointWorld(a.GetWorldPosition(), b.GetCentralSegment());

			Sphere s{ b.GetRadius() };
			s.SetRelativePosition(pt_s);
			s.UpdateWorldTransform();

			return Collide(a, s);
		}

		// TODO: Not yet implemented. Use SAT or simpler.
		static ContactManifold Collide(Sphere const& a, Hull const& b) { return ContactManifold{}; }

		static ContactManifold Collide(Capsule const& a, Capsule const& b) {
			Float32 const a_radius = a.GetRadius();
			Float32 const b_radius = b.GetRadius();

			auto [pa, pb, d2] = detail::ClosestPoints(a.GetCentralSegment(), b.GetCentralSegment());
			if (d2 > (a_radius + b_radius) * (a_radius + b_radius)) {
				return ContactManifold{
					.normal = d2 > 0.0001f ? (pb - pa) / std::sqrt(d2) : Vec3(NAN)
				};
			}

			Sphere sa{ a_radius }, sb{ b_radius };
			
			sa.SetRelativePosition(pa); // owner is world
			sa.UpdateWorldTransform();
			
			sb.SetRelativePosition(pb); // owner is world
			sb.UpdateWorldTransform();

			return Collide(sa, sb);
		}


		// TODO : not yet implemented. Use SAT or simpler.
		static ContactManifold Collide(Capsule const& a, Hull const& b) { return ContactManifold{}; }


		static ContactManifold Collide(Hull const& a, Hull const& b) {
			detail::FaceQuery fq_a = detail::SATQueryFaceDirections(a, b);
			fq_a.normal = b.LocalToWorldVec(fq_a.normal); // transform normal back to world space from a->b
			if (fq_a.separation > 0.0f) {
				return ContactManifold{
					.normal = fq_a.normal
				}; // no contacts, but track the separating normal
			}

			detail::FaceQuery fq_b = detail::SATQueryFaceDirections(b, a);
			fq_b.normal = -1.0f * a.LocalToWorldVec(fq_b.normal); // transform normal back to world space from a->b
			if (fq_b.separation > 0.0f) {
				return ContactManifold{
					.normal = fq_b.normal
				}; // no contacts, but track the separating normal
			}

			detail::EdgeQuery eq = detail::SATQueryEdgeDirections(a, b);
			eq.normal = b.LocalToWorldVec(eq.normal); // transform normal back to world space from a->b
			if (eq.separation > 0.0f) {
				return ContactManifold{
					.normal = eq.normal
				}; // no contacts, but track the separating normal
			}

			// Now generate the contact manifold!
			ContactManifold result{.num_contacts = 1};

			Vec3 normal = fq_a.normal;
			Float32 sep = std::max(fq_a.separation, std::max(fq_b.separation, eq.separation));
			if (sep == fq_b.separation) {
				normal = fq_b.normal;
			}
			else if (sep == eq.separation) {
				normal = eq.normal;
			}
			result.normal = normal;

			// TODO finish generating contacts
			// ...

			return result;
		}



		// ----------------------------
		// Swapped args implementations
		// ----------------------------
		// Helper for reversed arg order implementations of Collide
		template<typename U, typename T>
		static ContactManifold CollideSwapped(T const& a, U const& b) {
			ContactManifold m = Collide(b, a);
			m.normal *= -1.0f;
			return m;
		}
		static ContactManifold Collide(Hull const& a, Capsule const& b) { return detail::CollideSwapped(a, b); }
		static ContactManifold Collide(Hull const& a, Sphere const& b) { return detail::CollideSwapped(a, b); }
		static ContactManifold Collide(Capsule const& a, Sphere const& b) { return detail::CollideSwapped(a, b); }

	}

	////////////////////////////////////////////////////////////////////////////
	// COLLISION DISPATCH
	////////////////////////////////////////////////////////////////////////////

	namespace detail {
		template<typename T, typename U>
		Bool IntersectDispatch(Collider const* a, Collider const* b) {
			if (not a || not b) {
				SIK_ASSERT(false, "One of the pointers was null");
				return false;
			}
			return Intersect(*static_cast<T const*>(a), *static_cast<U const*>(b));
		}

		template<typename T, typename U>
		ContactManifold CollisionDispatch(Collider const* a, Collider const* b) {
			if (not a || not b) {
				SIK_ASSERT(false, "One of the pointers was null");
				return ContactManifold{};
			}
			return Collide(*static_cast<T const*>(a), *static_cast<U const*>(b));
		}
	}
#define INTERSECT_TABLE_ROW(Type) { &detail::IntersectDispatch<Type, Sphere>, &detail::IntersectDispatch<Type, Capsule>, &detail::IntersectDispatch<Type, Hull> }

	Bool Collider::BoundsIntersect(Collider const* other) const {
		using BVIntersectFn = Bool(*)(Collider const* a, Collider const* b);

		static constexpr BVIntersectFn intersect_table[4][4] = {
			INTERSECT_TABLE_ROW(Sphere),
			INTERSECT_TABLE_ROW(Capsule),
			INTERSECT_TABLE_ROW(Hull)
		};

		return intersect_table[GetTypeIdx()][other->GetTypeIdx()](this, other);
	}

#undef INTERSECT_TABLE_ROW
#define COLLIDE_TABLE_ROW(Type) { &detail::CollisionDispatch<Type, Sphere>, &detail::CollisionDispatch<Type, Capsule>, &detail::CollisionDispatch<Type, Hull> }

	ContactManifold Collider::Collide(Collider const* other) const {
		using CollideFn = ContactManifold(*)(Collider const* a, Collider const* b);

		static constexpr CollideFn collide_table[4][4] = {
			COLLIDE_TABLE_ROW(Sphere),
			COLLIDE_TABLE_ROW(Capsule),
			COLLIDE_TABLE_ROW(Hull)
		};

		return collide_table[GetTypeIdx()][other->GetTypeIdx()](this, other);
	}
#undef COLLIDE_TABLE_ROW



	////////////////////////////////////////////////////////////////////////////
	// HELPER IMPLEMENTATIONS
	////////////////////////////////////////////////////////////////////////////

	//--------------------------------------------------------------------------------------------------
	// Note the code below is adapted from Dirk Gregorius' 2013 GDC talk: "The Separating Axis Test"
	//
	// Copyright(C) 2012 by D. Gregorius. All rights reserved.
	//--------------------------------------------------------------------------------------------------
	namespace detail {
		static FaceQuery SATQueryFaceDirections(const Hull& hull1, const Hull& hull2) {

			// We perform all computations in local space of the second hull
			Mat4 const transform = glm::inverse(hull2.GetLocalToWorldTransform()) * hull1.GetLocalToWorldTransform();

			FaceQuery result{};

			Int32 const face_count = static_cast<Int32>(hull1.faces.size());
			for (Int32 i = 0; i < face_count; ++i)
			{
				Plane const p{ hull1.planes[i].Transform(transform) };

				Float32 const separation = Project(p, hull2);
				if (separation > result.separation)
				{
					result.index = i;
					result.separation = separation;
					result.normal = p.normal;
				}
			}
			return result;
		}

		// TODO : this should not have to be in world space!
		static Float32 Project(const Plane& plane, const Hull& hull) {
			Vec3 const support = hull.GetSupport( -plane.normal );
			return SignedDist(support, plane);

		}

		static inline Bool IsMinkowskiFace(Vec3 const& a, Vec3 const& b,
			Vec3 const& b_x_a,
			Vec3 const& c, Vec3 const& d,
			Vec3 const& d_x_c)
		{
			// Test if arcs AB and CD intersect on the unit sphere 
			Float32 const CBA = glm::dot(c, b_x_a);
			Float32 const DBA = glm::dot(d, b_x_a);
			Float32 const ADC = glm::dot(a, d_x_c);
			Float32 const BDC = glm::dot(b, d_x_c);

			return CBA * DBA < 0.0f && ADC * BDC < 0.0f && CBA * BDC > 0.0f;
		}

		static EdgeQuery SATQueryEdgeDirections(Hull const& hull1, Hull const& hull2) {

			// We perform all computations in local space of the second hull
			Mat4 const transform = glm::inverse(hull2.GetLocalToWorldTransform()) * hull1.GetLocalToWorldTransform();

			// Transform reference center of the first hull into local space of the second hull
			Vec3 const c1 = hull2.WorldToLocal( hull1.GetWorldPosition() );

			// Find axis of minimum penetration
			EdgeQuery result{};

			Int32 const edge_count1 = static_cast<Int32>(hull1.edges.size());
			Int32 const edge_count2 = static_cast<Int32>(hull2.edges.size());

			for (Int32 i = 0; i < edge_count1; i += 2)
			{
				Hull::HalfEdge const& edge1 = hull1.edges[i];
				Hull::HalfEdge const& twin1 = hull1.edges[i + 1];
				SIK_ASSERT(edge1.twin == i + 1 && twin1.twin == i, "Hull1 is invalid.");

				Vec3 const p1 = transform * Vec4(hull1.vertices[edge1.origin], 1);
				Vec3 const q1 = transform * Vec4(hull1.vertices[twin1.origin], 1);
				Vec3 const e1 = q1 - p1;

				Vec3 const u1 = Mat3(transform) * hull1.planes[edge1.face].normal;
				Vec3 const v1 = Mat3(transform) * hull1.planes[twin1.face].normal;

				for (Int32 j = 0; j < edge_count2; j += 2)
				{
					Hull::HalfEdge const& edge2 = hull2.edges[j];
					Hull::HalfEdge const& twin2 = hull2.edges[j + 1];
					SIK_ASSERT(edge2.twin == j + 1 && twin2.twin == j, "Hull2 is invalid.");

					Vec3 const p2 = Vec4(hull2.vertices[edge2.origin], 1);
					Vec3 const q2 = Vec4(hull2.vertices[twin2.origin], 1);
					Vec3 const e2 = q2 - p2;

					Vec3 const u2 = hull2.planes[edge2.face].normal;
					Vec3 const v2 = hull2.planes[twin2.face].normal;

					if (IsMinkowskiFace(u1, v1, -e1, -u2, -v2, -e2))
					{
						Vec3 normal{};
						Float32 const separation = Project(p1, e1, p2, e2, c1, normal);
						if (separation > result.separation)
						{
							result.index1 = i;
							result.index2 = j;
							result.separation = separation;
							result.normal = normal;
						}
					}
				}
			}

			return result;
		}

		static Float32 Project(Vec3 const& p1, Vec3 const& e1,
			Vec3 const& p2, Vec3 const& e2,
			Vec3 const& c1,
			Vec3& n) {

			// Build search direction
			Vec3 const e1_x_e2 = glm::cross(e1, e2);

			// Skip near parallel edges: |e1 x e2| = sin(alpha) * |e1| * |e2|
			static constexpr Float32 k_tolerance = 0.005f;
			Float32 const L = glm::length(e1_x_e2);
			if (L < k_tolerance * std::sqrt(glm::length2(e1) * glm::length2(e2))) {
				n = Vec3(NAN);
				return std::numeric_limits<Float32>::lowest();
			}

			// Assure consistent normal orientation (here: Hull1 -> Hull2)
			n = e1_x_e2 / L;
			if (glm::dot(n, p1 - c1) < 0.0f) {
				n *= -1.0f;
			}

			// s = Dot(n, p2) - d = Dot(n, p2) - Dot(n, p1) = Dot(n, p2 - p1) 
			return glm::dot(n, p2 - p1);
		}
	}
}

BEGIN_ATTRIBUTES_FOR(SphereArgs)
DEFINE_MEMBER(Float32, radius)
END_ATTRIBUTES


BEGIN_ATTRIBUTES_FOR(CapsuleArgs)
DEFINE_MEMBER(Float32, length)
DEFINE_MEMBER(Float32, radius)
END_ATTRIBUTES

BEGIN_ATTRIBUTES_FOR(HullArgs)
DEFINE_MEMBER(Vec3, halfwidths)
DEFINE_MEMBER(Bool, is_box)
END_ATTRIBUTES

BEGIN_ATTRIBUTES_FOR(ColliderCreationSettings)
DEFINE_MEMBER(Vec3, position_offset)
DEFINE_MEMBER(Quat, orientation_offset)
DEFINE_MEMBER(Float32, mass)
END_ATTRIBUTES