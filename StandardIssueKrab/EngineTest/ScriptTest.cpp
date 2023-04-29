#include "stdafx.h"

#include "ScriptTest.h"

#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/InputManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/AudioManager.h"
#include "Engine/GraphicsManager.h"


// TODO (bug) : this test leaks 120 bytes. So does SerializeTest (264 bytes) and
// GOAndCompTest (200 bytes). Maybe they're related...

void ScriptTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_scripting_manager = _p_engine_export_struct->p_engine_scripting_manager;
	p_audio_manager = _p_engine_export_struct->p_engine_audio_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif
	if (p_factory) {
		SetRunning();

		SIK_INFO("Creating game object");
		p_obj = p_factory->BuildGameObject("BehaviourObject2.json");

		SIK_INFO("Checking for Behaviour Component");
		if (p_obj->HasComponent<Behaviour>() != nullptr) {
			SIK_INFO("Found");
		}
		else {
			SIK_INFO("Not found");
			SetFailed();
		}

		return;
	}
	else {
		SIK_ERROR("Failed to initialize factory");
		SetError();
		return;
	}
}

void ScriptTest::Run() {
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SIK_INFO("Ending script test");
		SetPassed();
		return;
	}

	SetRunning();
	return;
}

void ScriptTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
