#pragma once

class Texture;

struct MaterialParameter
{
	// TODO Fill In Material Parameter
};

class Material
{
public:
	Vec3 diffuse;
	Vec3 specular;
	Vec3 emission;
	Float32 glossiness;
	GLuint* shader = 0;
	Texture* base_color = nullptr;
	Texture* normal_map = nullptr;

	void Use();
	void Unuse();

	static Material* bound_material;
private:	
	//Vector<MaterialParameter> m_params{};
};