#include "stdafx.h"

#include "SceneLoadTest.h"

#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/AudioManager.h"
#include "Engine/PhysicsManager.h"

/* TODO: This test leaks 144 bytes
*/

void SceneLoadTest::Setup(EngineExport* _p_engine_export_struct) {
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_scripting_manager = _p_engine_export_struct->p_engine_scripting_manager;
	p_audio_manager = _p_engine_export_struct->p_engine_audio_manager;
	p_physics_manager = _p_engine_export_struct->p_engine_physics_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

	if (p_factory) {
		SetRunning();

		SIK_INFO("Creating game objects");
		Vector<GameObject*> test = p_factory->BuildScene("TestScene.json");

		return;
	}
	else {
		SIK_ERROR("Failed to initialize factory");
		SetError();
		return;
	}
}

void SceneLoadTest::Run() {
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SIK_INFO("Ending script test");
		SetPassed();
		return;
	}

	SetRunning();
	return;
}

void SceneLoadTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
