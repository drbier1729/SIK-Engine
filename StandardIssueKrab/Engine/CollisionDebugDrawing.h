#pragma once

class Mesh;

namespace Collision {

	struct AABB;
	struct Plane;
	class Sphere;
	class Capsule;
	class Hull;

	class WireframeMesh {

	private:
		Vector<Vec3> line_start_end_pairs{};
		GLuint vbo = 0, vao = 0;
		Mesh const* p_underlying_mesh = nullptr; // non-owning reference!
		Bool locked = false;

	public:
		WireframeMesh() = default;
		explicit WireframeMesh(AABB const& box);
		explicit WireframeMesh(Plane const& plane);
		explicit WireframeMesh(Sphere const& sphere);
		explicit WireframeMesh(Capsule const& capsule);
		explicit WireframeMesh(Hull const& hull);
		~WireframeMesh() noexcept;
		WireframeMesh(WireframeMesh&&) noexcept;
		WireframeMesh& operator=(WireframeMesh&&) noexcept;

		// Non-copyable
		WireframeMesh(const WireframeMesh&) = delete;
		WireframeMesh& operator=(const WireframeMesh&) = delete;

		// Building interface
		void AddLine(Vec3 const& start, Vec3 const& end);
		void FinalizeLines();
		void Clear();
		inline Bool IsLocked() const { return locked; }

		// Rendering interface
		void Draw(GLuint shader, Mat4 const& model_tr = Mat4(1)) const;
	};
}