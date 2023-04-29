#pragma once

class Mesh;
class Material;
class GameObject;

struct MeshRenderer
{
public:
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	GameObject* owner = nullptr;
	bool is_valid = false;
	bool enabled = true;
	void Use();
	void Unuse();
	void Draw();
};