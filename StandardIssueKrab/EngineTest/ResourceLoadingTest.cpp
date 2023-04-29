#include "stdafx.h"
#include "ResourceLoadingTest.h"

#include "Engine/Texture.h"
#include "Engine/Mesh.h"
#include "Engine/StringID.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in ResourceLoadingTest.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }


#define CHECK(expr) if(not (expr) ) { SIK_ERROR("Check failed {}", #expr); SetError(); }

void ResourceLoadingTest::Setup(EngineExport* p_engine_export_struct) {
	rm = std::make_unique<ResourceManager>();
	if (rm == nullptr) { SetError(); }

#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
	if (p_dbg_string_dictionary == nullptr) { SetError(); return; }
#endif


	SetRunning();

	// Synchronous tests...

	// -------------------------------------------------------------------------
	// Textures
	// -------------------------------------------------------------------------
	Texture* goose_tex = rm->LoadTexture("goose.jpg");
	CHECK(goose_tex != nullptr)
	textures.push_back("goose.jpg"_sid);
	CHECK(goose_tex == rm->GetTexture("goose.jpg"_sid))

	Texture* digi_logo_tex = rm->LoadTexture("DigiPen_RGB_White.png");
	CHECK(digi_logo_tex != nullptr)
	textures.push_back("DigiPen_RGB_White.png"_sid);
	CHECK(digi_logo_tex == rm->GetTexture("DigiPen_RGB_White.png"_sid))


	// -------------------------------------------------------------------------
	// Meshes
	// -------------------------------------------------------------------------
	/*Mesh* sph = rm->LoadMesh("sphere.fbx");
	CHECK(sph != nullptr)
	meshes.push_back("sphere.fbx"_sid);
	CHECK(sph == rm->GetMesh("sphere.fbx"_sid))*/

	Mesh* sph2 = rm->LoadMesh("sphere_primitive.fbx");
	CHECK(sph2 != nullptr)
	meshes.push_back("sphere_primitive.fbx"_sid);
	CHECK(sph2 == rm->GetMesh("sphere_primitive.fbx"_sid))


	// -------------------------------------------------------------------------
	// Shaders
	// -------------------------------------------------------------------------
	GLuint* lit = rm->LoadShaderProgram("lit");
	CHECK(lit != 0)
	shaders.push_back("lit"_sid);
	CHECK(lit == rm->GetShaderProgram("lit"_sid))

	GLuint* bg = rm->LoadShaderProgram("background");
	CHECK(bg != 0)
	shaders.push_back("background"_sid);
	CHECK(bg == rm->GetShaderProgram("background"_sid))


	// -------------------------------------------------------------------------
	// JSONs
	// -------------------------------------------------------------------------
	JSON* test_obj = rm->LoadJSON("TestObject.json");
	CHECK(test_obj != nullptr)
	jsons.push_back("TestObject.json"_sid);
	CHECK(test_obj == rm->GetJSON("TestObject.json"_sid));

	JSON* bh_obj = rm->LoadJSON("BehaviourObject.json");
	CHECK(bh_obj != nullptr)
	jsons.push_back("BehaviourObject.json"_sid);
	CHECK(bh_obj == rm->GetJSON("BehaviourObject.json"_sid));


	// Start Async test
	gui_shader = rm->AsyncLoadShaderProgram("gui");
	shaders.push_back("gui"_sid);
	
	big_tex = rm->AsyncLoadTexture("Tropical_Beach_8k.jpg");
	textures.push_back("Tropical_Beach_8k.jpg"_sid);
	
	sph_mesh = rm->AsyncLoadMesh("sphere.fbx");
	meshes.push_back("sphere.fbx"_sid);
	
	scene_json = rm->AsyncLoadJSON("TestScene.json");
	meshes.push_back("TestScene.json"_sid);


	CHECKERROR
}

void ResourceLoadingTest::Run() {
	static Uint32 frame_counter = 0;


	CHECKERROR

	SetRunning();

	if (gui_shader != nullptr) { 
		if (not shader_loaded) { 
			if (*gui_shader == rm->GetDefaultShaderProgram()) { // still using default shader
				if (frame_counter % 120 == 0) {
					SIK_WARN("Waiting for GUI shader to be assigned...")
				}
			}
			else {
				SIK_INFO("Success! After {} frames, GUI shader loaded: {}", frame_counter, *gui_shader);
				shader_loaded = true;
			}
		}
	}

	if (big_tex != nullptr) {
		if (not tex_loaded) {
			if (big_tex->texture_id == 0) { // still using default texture
				if (frame_counter % 120 == 0) {
					SIK_WARN("Waiting for Big Texture to be assigned...")
				}
			}
			else {
				SIK_INFO("Success! After {} frames, Big Texture loaded: width = {}, height = {}, id = {}", frame_counter, big_tex->width, big_tex->height, big_tex->texture_id);
				tex_loaded = true;
			}
		}
	}
	
	if (sph_mesh != nullptr) {
		if (not mesh_loaded) {
			if (sph_mesh->GetVertexArray() == 0) { // still using default mesh
				if (frame_counter % 120 == 0) {
					SIK_WARN("Waiting for Sphere Mesh to be assigned...")
				}
			}
			else {
				SIK_INFO("Success! After {} frames, Sphere Mesh loaded: VAO = {}, Vert Count = {}, Index Count = {}", frame_counter, sph_mesh->GetVertexArray(), sph_mesh->GetVertexCount(), sph_mesh->GetIndexCount());
				mesh_loaded = true;
			}
		}
	}
	
	if (scene_json != nullptr) {
		if (not json_loaded) {
			if (scene_json->path.string() == "") { // still using default JSON
				if (frame_counter % 120 == 0) {
					SIK_WARN("Waiting for Scene JSON to be assigned...")
				}
			}
			else {
				SIK_INFO("Success! After {} frames, Scene JSON loaded: path = \"{}\"", frame_counter, scene_json->path.string().c_str());
				json_loaded = true;
			}
		}
	}

	if (shader_loaded && tex_loaded && mesh_loaded) { frame_counter = 0; SetPassed(); }

	rm->ProcessAssetsFor(4ms);
	frame_counter++;
}

void ResourceLoadingTest::Teardown() {

	CHECKERROR
	// Make sure that everything can be unloaded
	for (auto&& t : textures) {
		rm->UnloadTexture(t);
		CHECK(rm->GetTexture(t) == nullptr);
	}

	CHECKERROR
	for (auto&& m : meshes) {
		rm->UnloadMesh(m);
		CHECK(rm->GetMesh(m) == nullptr);
	}

	CHECKERROR
	for (auto&& s : shaders) {

		rm->UnloadShaderProgram(s);
		CHECK(rm->GetShaderProgram(s) == 0);
	}

	CHECKERROR
	for (auto&& j : jsons) {
		rm->UnloadJSON(j);
		CHECK(rm->GetJSON(j) == nullptr);
	}

	CHECKERROR
	SetPassed();
}

#undef CHECK