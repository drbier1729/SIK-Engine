#pragma once
/****************************************************************************** /
/* !
/* File Mesh.h
/* Author Andrew Rudasics
/* Email: andrew.rudasics@digipen.edu
/* Date 9/24/2022
/* Interface for mesh class used to represent mesh vertex data.
/* DigiPen Institute of Technology © 2022
/******************************************************************************/

/*
* The Mesh class encapsulates the mesh data storage of objects.
* Provides methods to add and access data.
*/
class Mesh
{
public:
	using Ref = std::reference_wrapper<Mesh>;
	using CRef = std::reference_wrapper<const Mesh>;

private:
	GLuint m_vao = 0, m_vbo = 0, m_ebo = 0, m_tan_vbo;
	SizeT m_num_vertices = 0, m_num_indices = 0;

public:
	Mesh() = default;
	Mesh(const Float32* verts, SizeT num_verts,
		const Uint32* indx, SizeT num_indices);
	Mesh(const Float32* verts, SizeT num_verts, 
		const Uint32* indx, SizeT num_indices,
		const Float32* tangents, SizeT num_tangents);

	// Non-copyable. Use Ref or CRef instead.
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;

	~Mesh() noexcept;

	void Use() const;
	void Unuse() const;
	void Draw() const;

	inline GLuint GetVertexArray() const;
	inline SizeT  GetVertexCount() const;
	inline SizeT  GetIndexCount() const;

private:
	static GLuint  default_vao;
	static GLsizei default_num_idxs;
	static GLuint  bound_vao;
	static UniquePtr<Mesh> plane;
	static UniquePtr<Mesh> cube;
	static UniquePtr<Mesh> sphere;
	static UniquePtr<Mesh> cone;
	static UniquePtr<Mesh> triangle;
	static UniquePtr<Mesh> cylinder;

	static void InitPlane();
	static void InitCube();
	static void InitSphere();
	static void InitCone();
	static void InitTriangle();
	static void InitCylinder();

	GLuint GenerateTangents(const Float32* verts, const Uint32* indx);
public:
	static void InitDefaults();
	static void FreeDefaults();

	inline static Mesh const& Plane();
	inline static Mesh const& Cube();
	inline static Mesh const& Sphere();
	inline static Mesh const& Cone();
	inline static Mesh const& Triangle();
	inline static Mesh const& Cylinder();

	// TODO : it would be nice to make these references
	// so no one can accidentally fuck up the globals.
	inline static Mesh* PlanePtr();
	inline static Mesh* CubePtr();
	inline static Mesh* SpherePtr();
	inline static Mesh* ConePtr();
	inline static Mesh* CylinderPtr();
};


// Inline definitions
inline GLuint Mesh::GetVertexArray() const {
	return m_vao;
}

inline SizeT Mesh::GetVertexCount() const {
	return m_num_vertices;
}

inline SizeT Mesh::GetIndexCount() const {
	return m_num_indices;
}

inline Mesh* Mesh::PlanePtr() {
	return plane.get();
}

inline Mesh* Mesh::CubePtr() {
	return cube.get();
}

inline Mesh* Mesh::SpherePtr() {
	return sphere.get();
}

inline Mesh* Mesh::ConePtr() {
	return cone.get();
}

inline Mesh* Mesh::CylinderPtr() {
	return cylinder.get();
}

inline Mesh const& Mesh::Plane() {
	return *plane;
}

inline Mesh const& Mesh::Cube() {
	return *cube;
}

inline Mesh const& Mesh::Sphere() {
	return *sphere;
}

inline Mesh const& Mesh::Cone() {
	return *cone;
}

inline Mesh const& Mesh::Triangle() {
	return *triangle;
}

inline Mesh const& Mesh::Cylinder() {
	return *cylinder;
}
