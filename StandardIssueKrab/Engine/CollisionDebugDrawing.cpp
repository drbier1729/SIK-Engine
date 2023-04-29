#include "stdafx.h"
#include "CollisionDebugDrawing.h"
#include "Collision.h"
#include "Mesh.h"


#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }


namespace Collision {

	WireframeMesh::WireframeMesh(WireframeMesh&& src) noexcept
		: vao{ src.vao }, vbo{ src.vbo }, p_underlying_mesh{ src.p_underlying_mesh },
		line_start_end_pairs{ std::move(src.line_start_end_pairs) }
	{
		src.vao = 0;
		src.vbo = 0;
		src.locked = false;
		src.p_underlying_mesh = nullptr;
	}

	WireframeMesh& WireframeMesh::operator=(WireframeMesh&& rhs) noexcept {
		if (&rhs == this) {
			return *this;
		}

		if (locked) {
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
		}

		vao = rhs.vao;
		vbo = rhs.vbo;
		line_start_end_pairs = std::move(rhs.line_start_end_pairs);
		p_underlying_mesh = rhs.p_underlying_mesh;
		locked = rhs.locked;

		rhs.vao = 0;
		rhs.vbo = 0;
		rhs.locked = false;
		rhs.p_underlying_mesh = nullptr;

		return *this;
	}

	WireframeMesh::~WireframeMesh() noexcept {
		if (locked) {
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
		}
	}

	void WireframeMesh::AddLine(Vec3 const& start, Vec3 const& end) {
		SIK_ASSERT(not locked, "This mesh has already been finalized. You must call Clear() to reuse.");
		if (locked) { return; }

		line_start_end_pairs.push_back(start);
		line_start_end_pairs.push_back(end);
	}

	void WireframeMesh::FinalizeLines() {
		if (locked) { return; }
		locked = true;

		if (line_start_end_pairs.empty()) { return; }

		// Generate vertex array
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			line_start_end_pairs.size() * sizeof(decltype(line_start_end_pairs)::value_type),
			line_start_end_pairs.data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		CHECKERROR
	}

	void WireframeMesh::Draw(GLuint active_shader, Mat4 const& model_tr) const {

		int loc = glGetUniformLocation(active_shader, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(model_tr[0][0]));
		
		if (p_underlying_mesh) {
			p_underlying_mesh->Use();
			p_underlying_mesh->Draw();
			p_underlying_mesh->Unuse();
		}
		if (vao != 0) {
			glBindVertexArray(vao);
			glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_start_end_pairs.size()));
			glBindVertexArray(0);
		}

		CHECKERROR
	}

	void WireframeMesh::Clear() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);

		line_start_end_pairs.clear();
		vbo = 0;
		vao = 0;
		locked = false;
	}


	WireframeMesh::WireframeMesh(AABB const& box) 
		: WireframeMesh()
	{
		p_underlying_mesh = Mesh::CubePtr();
		SIK_ASSERT(p_underlying_mesh != nullptr, "No static Box mesh found. Perhaps it has not been initialized yet. See Mesh.h");
	}


	WireframeMesh::WireframeMesh(Sphere const& sphere) 
		: WireframeMesh()
	{
		p_underlying_mesh = Mesh::SpherePtr();
		SIK_ASSERT(p_underlying_mesh != nullptr, "No static Sphere mesh found. Perhaps it has not been initialized yet. See Mesh.h");
	}

	WireframeMesh::WireframeMesh(Hull const& hull) 
		: WireframeMesh()
	{
		// Note that Hull class guarantees less than 256 edges
		Uint8 const num_edges = static_cast<Uint8>(hull.edges.size());

		for (Uint8 i = 0; i < num_edges; i += 2) {
			auto const& e = hull.edges[i];
			SIK_ASSERT(e.twin == i + 1 && hull.edges[i + 1].twin == i, "Hull is invalid. Twin edges must be adjacent.");

			Vec3 const start = hull.vertices[e.origin];
			Vec3 const end = hull.vertices[hull.edges[e.next].origin];
			AddLine(start, end);
		}
	}

	WireframeMesh::WireframeMesh(Plane const& plane) 
		: WireframeMesh()
	{
		// Normal
		AddLine(Vec3(0), plane.normal * 4.0f);

		// Plane
		LineSegment seg1{ .start = Vec3(0, -4, -4), .end = Vec3(0, -4, 4) };
		LineSegment seg2{ .start = Vec3(0, -4, 4), .end = Vec3(0, 4, 4) };
		LineSegment seg3{ .start = Vec3(0, 4, 4), .end = Vec3(0, 4, -4) };
		LineSegment seg4{ .start = Vec3(0, 4, -4), .end = Vec3(0, -4, -4) };
		LineSegment seg5{ .start = Vec3(0, -4, -4), .end = Vec3(0, 4, 4) };
		LineSegment seg6{ .start = Vec3(0, -4, 4), .end = Vec3(0, 4, -4) };

		AddLine(seg1.start, seg1.end);
		AddLine(seg2.start, seg2.end);
		AddLine(seg3.start, seg3.end);
		AddLine(seg4.start, seg4.end);
		AddLine(seg5.start, seg5.end);
		AddLine(seg6.start, seg6.end);
	}

	WireframeMesh::WireframeMesh(Capsule const& capsule) 
		: WireframeMesh()
	{
		// All capsules are oriented vertically in local space
		static constexpr Vec3 normal = Vec3(0, 1, 0);
		static constexpr Vec3 tangent = Vec3(0, 0, 1);

		Float32 const halflength = capsule.GetLength() * 0.5f;
		Float32 const radius = capsule.GetRadius();

		Vec3 const start = -halflength * normal;
		Vec3 const end = halflength * normal;

		// Generate points on central segment
		Array<Vec3, 7> seg_pts = {
			start - 0.66f * normal * radius,
			start - 0.33f * normal * radius,
			start,
			Vec3(0),
			end,
			end + 0.33f * normal * radius,
			end + 0.66f * normal * radius
		};

		// Use central segment to generate points on boundary
		Array<Vec3, 7 * 8> boundary_pts;
		for (auto i = 0u; i < 8; ++i) {

			Vec3 const offset = radius * Mat3(glm::rotate(i / 8.0f * 6.28f, normal)) * tangent;

			// Bottom hemisphere
			boundary_pts[i * 7 + 0] = seg_pts[0] + 0.577f * offset;
			boundary_pts[i * 7 + 1] = seg_pts[1] + 0.816f * offset;

			// Central cylinder
			boundary_pts[i * 7 + 2] = seg_pts[2] + offset;
			boundary_pts[i * 7 + 3] = seg_pts[3] + offset;
			boundary_pts[i * 7 + 4] = seg_pts[4] + offset;

			// Top hemisphere
			boundary_pts[i * 7 + 5] = seg_pts[5] + 0.816f * offset;
			boundary_pts[i * 7 + 6] = seg_pts[6] + 0.577f * offset;
		}

		// Define some helpers for getting neighbors of a point
		auto up_neighbor = [&](Uint32 idx) {
			if (idx % 7 == 6) {
				return end + normal * radius;
			}
			return boundary_pts[idx + 1];
		};

		auto dn_neighbor = [&](Uint32 idx) {
			if (idx % 7 == 0) {
				return start - normal * radius;
			}
			return boundary_pts[idx - 1];
		};

		auto r_neighbor = [&](Uint32 idx) {
			return boundary_pts[(idx + 7) % (7 * 8)];
		};

		auto l_neighbor = [&](Uint32 idx) {
			return (idx < 7) ?
				boundary_pts[7 * 7 + idx] :
				boundary_pts[idx - 7];
		};

		// Connect boundary points to neighbors
		for (auto i = 0u; i < 7 * 8; ++i) {
			AddLine(boundary_pts[i], r_neighbor(i));
			AddLine(boundary_pts[i], up_neighbor(i));
			AddLine(boundary_pts[i], l_neighbor(i));
			AddLine(boundary_pts[i], dn_neighbor(i));
		}
	}


	void AABB::DebugDraw(GLuint shader, Quat const& rot) const {
		Mat4 const model_tr = glm::translate(position) * glm::toMat4(rot) * glm::scale(halfwidths);

		int loc = glGetUniformLocation(shader, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(model_tr[0][0]));

		Mesh const& cube_mesh = Mesh::Cube();
		
		cube_mesh.Use();
		cube_mesh.Draw();
		cube_mesh.Unuse();
	}
}