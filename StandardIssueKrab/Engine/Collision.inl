
namespace Collision {

	// Helpers
	namespace detail {
		inline Bool EpsilonEqual(float a, float b)
		{
			static constexpr float epsilon = std::numeric_limits<float>::epsilon();

			return std::fabs(a - b) <= std::max({ std::fabs(a), std::fabs(b), 1.0f }) * std::sqrtf(epsilon);
		}

		inline Vec3 GetAnyUnitOrthogonalTo(Vec3 const& src) {
			Vec3 other;
			if (glm::epsilonNotEqual(src.y, 0.0f, 0.0001f) ||
				glm::epsilonNotEqual(src.z, 0.0f, 0.0001f)) {
				other = Vec3(1, 0, 0);
			}
			else {
				other = Vec3(0, 1, 0);
			}
			return glm::normalize(glm::cross(other, src));
		}

		inline Mat4 BasisFromUnitVector(Vec3 const& v) {
			SIK_ASSERT(glm::epsilonEqual(glm::length2(v), 1.0f, 0.001f), "v must be a unit vector");

			Vec3 const axis1 = detail::GetAnyUnitOrthogonalTo(v);
			Vec3 const axis2 = glm::cross(v, axis1);

			return Mat4{
				Vec4(v,0),
				Vec4(axis1,0),
				Vec4(axis2,0),
				Vec4(0,0,0,1)
			};
		}
	}


	// Ray Methods

	inline Ray::CastResult Ray::Cast(AABB const& aabb) const
	{
		Vec3 const min = aabb.Min();
		Vec3 const max = aabb.Max();

		Float32 tmin = 0.0f;
		Float32 tmax = std::numeric_limits<Float32>::max();

		// For all three slabs
		for (Uint32 i = 0u; i < 3u; ++i)
		{
			if (detail::EpsilonEqual(d[i], 0.0f))
			{
				// Ray is parallel to slab. No hit if origin not within slab
				if (p[i] < min[i] || p[i] > max[i]) { return Ray::CastResult{}; }
			}
			else {
				// Compute intersection t value of ray with near and far plane of slab
				Float32 const ood = 1.0f / d[i];
				Float32 t1 = (min[i] - p[i]) * ood;
				Float32 t2 = (max[i] - p[i]) * ood;

				// Make t1 be intersection with near plane, t2 with far plane
				if (t1 > t2) { std::swap(t1, t2); }

				// Compute the intersection of slab intersection intervals
				tmin = std::max(tmin, t1);
				tmax = std::min(tmax, t2);

				// Exit with no collision as soon as slab intersection becomes empty
				if (tmin > tmax) { return Ray::CastResult{}; }
			}
		}

		// Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
		return Ray::CastResult{
			.point = p + d * tmin,
			.distance = tmin,
			.hit = true
		};
	}


	// LineSegment Methods

	inline LineSegment LineSegment::Transform(Mat4 const& transform) const {
		return LineSegment{
			.start = transform * Vec4(start, 1),
			.end = transform * Vec4(end, 1)
		};
	}

	inline Vec3 LineSegment::Midpoint() const {
		return 0.5f * (start + end);
	}


	inline AABB& AABB::SetMinMax(Vec3 const& min, Vec3 const& max)
	{
		position = 0.5f * (max + min);
		halfwidths = 0.5f * (max - min);
		return *this;
	}


	// AABB Methods

	inline Bool AABB::Intersects(AABB const& other) const {
		return glm::all(
			glm::lessThanEqual(
				glm::abs(position - other.position),
				halfwidths + other.halfwidths
			)
		);
	}

	inline Bool AABB::Contains(AABB const& other) const
	{
		return glm::all(glm::lessThanEqual(Min(), other.Min())) &&
			glm::all(glm::greaterThanEqual(Max(), other.Max()));
	}

	inline Float32 AABB::DistSquaredFromPoint(Vec3 const& pt) const
	{
		Vec3 const min = Min();
		Vec3 const max = Max();
		Float32 d2 = 0.0f;
		for (Uint32 i = 0; i < 3; ++i)
		{
			Float32 const v = pt[i];
			if (v < min[i]) { d2 += (min[i] - v) * (min[i] - v); }
			if (v > max[i]) { d2 += (v - max[i]) * (v - max[i]); }
		}
		return d2;
	}

	inline Vec3 AABB::Min() const
	{
		return position - halfwidths;
	}

	inline Vec3 AABB::Max() const
	{
		return position + halfwidths;
	}

	inline Float32 AABB::SurfaceArea() const
	{
		Vec3 const d = 2.0f * halfwidths;
		return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
	}

	inline AABB AABB::Transformed(Mat4 const& transform) const {
		return Transformed(Mat3(transform), transform[3]);
	}

	inline AABB AABB::Transformed(Mat3 const& orientation, Vec3 const& pos) const
	{
		Vec3 max = pos, min = pos;

		Vec3 const thisMin = Min(), thisMax = Max();

		// Form extents by summing smaller and larger terms respectively
		for (auto i = 0; i < 3; i++) {
			for (auto j = 0; j < 3; j++) {

				Float32 const e = orientation[i][j] * thisMin[j];
				Float32 const f = orientation[i][j] * thisMax[j];

				if (e < f)
				{
					min[i] += e;
					max[i] += f;
				}
				else
				{
					min[i] += f;
					max[i] += e;
				}
			}
		}
		AABB result{};
		result.SetMinMax(min, max);
		return result;
	}

	inline AABB AABB::MovedBy(Vec3 const& displacement) const
	{
		return AABB{ .position = position + displacement, .halfwidths =  halfwidths };
	}

	inline AABB AABB::Expanded(Float32 const scaleFactor) const
	{
		Vec3 const r{ scaleFactor };
		return AABB{ .position = position, .halfwidths = halfwidths + r };
	}

	inline AABB AABB::Union(AABB const& other) const
	{
		Vec3 const min = glm::min(Min(), other.Min());
		Vec3 const max = glm::max(Max(), other.Max());
		
		return AABB{}.SetMinMax(min, max);
	}


	// Plane methods

	inline Plane Plane::Transform(Mat4 const& transform) const {
		Plane p{};
		p.normal = glm::normalize(Mat3(transform) * normal);
		p.d = glm::dot(Vec3(transform[3]), p.normal) + d;

		SIK_ASSERT(not glm::any(glm::isnan(p.normal)), "Bad transform matrix.");
		return p;
	}

	inline Plane Plane::Make(Vec3 const& normal, Vec3 const& pt) {
		return Plane{ .normal = normal, .d = glm::dot(normal, pt) };
	}


	// Collider methods
	inline void Collider::UpdateWorldTransformFromBody(Mat4 const& body_tr) {
		local_to_world = body_tr * local_to_body;
	}

	inline Collider::Type Collider::GetType() const {
		return type; }

	inline Uint32 Collider::GetTypeIdx() const { 
		return static_cast<Uint32>(type) - 1u; } // intentionally overflows NONE

	inline Mat3 Collider::GetInertiaTensorInSpace(Vec3 const& tran, Mat3 const& rot) const {
		// Rotated inertia tensor
		Mat3 I = rot * inertia_local * glm::transpose(rot);

		// Using Parallel Axis Theorem for translated inertia tensor
		I += (glm::dot(tran, tran) * Mat3(1) - glm::outerProduct(tran, tran));

		return I;
	}

	inline Vec3 Collider::LocalToBody(Vec3 const& pt) const {
		return Vec3( local_to_body * Vec4(pt, 1) );
	}

	inline Vec3 Collider::LocalToBodyVec(Vec3 const& vec) const {
		return Mat3(local_to_body) * vec;
	}

	inline Vec3 Collider::BodyToLocal(Vec3 const& pt) const {
		return Vec3( glm::inverse(local_to_body) * Vec4(pt, 1) );
	}

	inline Vec3 Collider::BodyToLocalVec(Vec3 const& vec) const {
		return glm::inverse(Mat3(local_to_body)) * vec;
	}

	inline Mat4 Collider::GetLocalToBodyTransform() const {
		return local_to_body;
	}

	inline Mat4 Collider::GetRelativeRotationMat4() const {
		Mat4 r = local_to_body;
		r[3] = Vec4(0, 0, 0, 1);
		return r;
	}

	inline Mat3 Collider::GetRelativeRotationMat3() const {
		return Mat3(local_to_body);
	}

	inline Vec3 Collider::GetRelativePosition() const {
		return Vec3(local_to_body[3]);
	}

	inline Mat3 Collider::GetInertiaTensorRelative() const {
		return GetInertiaTensorInSpace(GetRelativePosition(), GetRelativeRotationMat3());
	}

	inline Vec3 Collider::LocalToWorld(Vec3 const& pt) const {
		return Vec3( local_to_world * Vec4(pt, 1) );
	}

	inline Vec3 Collider::LocalToWorldVec(Vec3 const& vec) const {
		return Mat3(local_to_world) * vec;
	}

	inline Vec3 Collider::WorldToLocal(Vec3 const& pt) const {
		return glm::inverse(local_to_world) * Vec4(pt, 1);
	}
	
	inline Vec3 Collider::WorldToLocalVec(Vec3 const& vec) const {
		return glm::inverse(Mat3(local_to_world)) * vec;
	}

	inline Mat4 Collider::GetLocalToWorldTransform() const {
		return local_to_world;
	}
	
	inline Mat3 Collider::GetWorldRotationMat3() const {
		return Mat3(local_to_world);
	}
	
	inline Mat4 Collider::GetWorldRotationMat4() const {
		Mat4 r = local_to_world;
		r[3] = Vec4(0, 0, 0, 1);
		return r;
	}

	inline Vec3 Collider::GetWorldPosition() const {
		return local_to_world[3];

	}
	
	inline Mat3 Collider::GetInertiaTensorWorldRotation() const {
		Mat3 const rot = Mat3(local_to_world);
		return rot * inertia_local * glm::transpose(rot);
	}

	inline void Collider::SetLocalToBodyTransform(Mat4 const& tr) {
		local_to_body = tr;
	}

	inline void Collider::SetRelativeRotation(Mat3 const& rot) {
		local_to_body[0] = Vec4(rot[0], 0);
		local_to_body[1] = Vec4(rot[1], 0);
		local_to_body[2] = Vec4(rot[2], 0);
	}

	inline void Collider::SetRelativePosition(Vec3 const& pos) {
		local_to_body[3] = Vec4(pos, 1);
	}

	
	// Sphere methods
	inline void Sphere::ComputeInertiaTensor() {
		inertia_local = 0.4f * radius * radius * Mat3(1);
	}
	
	inline Float32 Sphere::GetRadius() const { 
		return radius; 
	}
	
	inline void Sphere::SetRadius(Float32 r) { 
		radius = r; 
		ComputeInertiaTensor(); 
	}


	// Capsule methods
	inline LineSegment Capsule::GetCentralSegment() const {
		LineSegment ls{ .start = Vec3(0, -seg_halflength, 0), .end = Vec3(0, seg_halflength, 0) };
		return ls.Transform( GetLocalToWorldTransform() );
	}

	inline void Capsule::ComputeInertiaTensor() {
		// Compute inertia tensor, assuming oriented vertically
		static constexpr Float32 two_pi = 6.283f;

		Float32 const rSq = radius * radius;
		Float32 const height = 2.0f * seg_halflength;

		// Cylinder volume
		Float32 const cV = two_pi * seg_halflength * rSq;

		// Hemisphere volume
		Float32 const hsV = two_pi * rSq * radius / 3.0f;

		Mat3 I(0);
		I[1][1] = rSq * cV * 0.5f;
		I[0][0] = I[2][2] = I[1][1] * 0.5f + cV * height * height / 12.0f;

		Float32 const temp0 = hsV * 2.0f * rSq / 5.0f;
		I[1][1] += temp0 * 2.0f;

		Float32 const temp1 = height * 0.5f;
		Float32 const temp2 = temp0 + hsV * temp1 * temp1 + 3.0f * height * radius / 8.0f;
		I[0][0] += temp2 * 2.0f;
		I[2][2] += temp2 * 2.0f;
		I[0][1] = I[0][2] = I[1][0] = I[1][2] = I[2][0] = I[2][1] = 0.0f;

		inertia_local = I;
	}

	inline Float32 Capsule::GetLength() const { 
		return 2.0f * seg_halflength; }
	
	inline Float32 Capsule::GetRadius() const { 
		return radius; }

	inline void Capsule::SetLength(Float32 len) { 
		seg_halflength = 0.5f * len; 
		ComputeInertiaTensor(); }

	inline void Capsule::SetRadius(Float32 r) { 
		radius = r; 
		ComputeInertiaTensor(); }


	// Hull methods
	inline Vec3 Hull::GetSupport(Vec3 const& directionL) const {
		Int32   max_index = -1;
		Float32 max_projection = std::numeric_limits<Float32>::lowest();

		Int32 const vert_count = static_cast<Int32>(vertices.size());

		for (Int32 i = 0; i < vert_count; ++i)
		{
			Float32 const projection = glm::dot(directionL, vertices[i]);
			if (projection > max_projection)
			{
				max_index = i;
				max_projection = projection;
			}
		}
		return vertices[max_index];
	}

	inline void Hull::Scale(Vec3 const& scale) {
		Scale(scale.x, scale.y, scale.z);
		ComputeInertiaTensor();
	}

	inline void Hull::Scale(Float32 x, Float32 y, Float32 z) {
		SIK_ASSERT(x > 0.001f && y > 0.001f && z > 0.001f, "Scale must be positive.");

		// Scale vertices
		for (auto&& v : vertices) {
			v.x *= x;
			v.y *= y;
			v.z *= z;
		}

		// Scale bounding box
		bounds.x *= x;
		bounds.y *= y;
		bounds.z *= z;

		// Scale face planes
		for (auto&& p : planes) {
			p.d = std::abs(x * p.normal.x + y * p.normal.y + z * p.normal.z);
		}
		ComputeInertiaTensor();
	}
}