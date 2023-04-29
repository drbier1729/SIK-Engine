#include "stdafx.h"
#include "Test.h"
#include "TextureTest.h"
#include "Engine/StringID.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/Texture.h"

/*
* Sets up the Texture test.
* Initializes the resource manager pointer
* Returns: TestState
*/
void TextureTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif
	SetRunning();
	return;
}

/*
* Runs the Texture test
* Steps:
* 1) Load a texture
* 2) Check size of memory resource
* 3) Load the same texture again
* 4) Verify that size of memory resource doesn't increase
* 5) Load a different texture
* 6) Check size of memory resource
* 7) Delete all textures
* 8) Verify size of memory resource is 0
* Returns: TestState
*/
void TextureTest::Run() {
	SIK_INFO("1. Loading the texture");
	Texture* digi_texture = p_resource_manager->LoadTexture("DigiPen_RGB_White.png");

	if (digi_texture == nullptr) {
		SIK_ERROR("Failed to load texture DigiPen_RGB_White");
		SetFailed();
		return;
	}


	SIK_INFO("2. Loading the same texture");
	Texture* digi_texture_copy = p_resource_manager->LoadTexture("DigiPen_RGB_White.png");

	SIK_INFO("3. Verify that same texture was loaded");
	if (digi_texture != digi_texture_copy) {
		SIK_ERROR("Different texures were loaded for the same file");
		SetFailed();
		return;
	}

	SIK_INFO("4. Loading the second texture");
	Texture* fmod_texture = p_resource_manager->LoadTexture("FMOD_Logo_White_Black_Background.png");

	if (fmod_texture == nullptr) {
		SIK_ERROR("Failed to load texture FMOD_Logo_White_Black_Background");
		SetFailed();
		return;
	}

	SIK_INFO("5. Get texture 1");
	digi_texture_copy = p_resource_manager->GetTexture("DigiPen_RGB_White.png");

	SIK_INFO("6. Verify that same texture was loaded");
	if (digi_texture != digi_texture_copy) {
		SIK_ERROR("Different texures were loaded for the same file");
		SetFailed();
		return;
	}

	SIK_INFO("7. Get texture 2");
	Texture* fmod_texture_copy = p_resource_manager->GetTexture("FMOD_Logo_White_Black_Background.png");

	SIK_INFO("11. Verify that same texture was loaded");
	if (fmod_texture != fmod_texture_copy) {
		SIK_ERROR("Different texures were loaded for the same file");
		SetFailed();
		return;
	}

	SetPassed();
	return;
}

/*
* Runs the teardown
* Deletes all the textures
* Returns: TestState
*/
void TextureTest::Teardown() {
	SetPassed();
	return;
}
