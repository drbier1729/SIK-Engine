#pragma once

// forward declaration
class Texture;

static constexpr Uint32 MAX_BONE_INFLUENCE = 4;

struct Vertex {
	Vec3 position;
	Vec3 normal;
	Vec2 tex_coords;
	Vec3 tangent;
	Vec3 bi_tangent;

	// bone indexes which influence this vertex
	Int32 bone_ids[MAX_BONE_INFLUENCE];
	// weights from each bone
	Float32 weights[MAX_BONE_INFLUENCE];
};

class SkinnedMesh {
public:
	// mesh data
	Vector<Vertex> vertices;
	Vector<Uint32> indices;
	Vector<Texture*> textures;
	Uint32 vao;

	SkinnedMesh(Vector<Vertex> vertices, Vector<Uint32> indices, Vector<Texture*> textures);

	void Draw(GLuint shader_program);

private:
	Uint32 vbo, ebo;

	void SetupMesh();
};