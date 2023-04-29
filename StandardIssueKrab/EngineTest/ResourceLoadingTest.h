#pragma once

#include "Test.h"
#include "Engine/ResourceManager.h"

class ResourceLoadingTest : public Test {

public:
	void Setup(EngineExport* p_engine_export_struct) override;
	void Run() override;
	void Teardown() override;

private:
	UniquePtr<ResourceManager> rm;

	GLuint* gui_shader = nullptr;
	class Texture* big_tex = nullptr;
	class Mesh* sph_mesh = nullptr;
	struct JSON* scene_json = nullptr;

	bool shader_loaded, tex_loaded, mesh_loaded, json_loaded;

	Vector<StringID> textures;
	Vector<StringID> shaders;
	Vector<StringID> meshes;
	Vector<StringID> jsons;
};