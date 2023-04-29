#include "stdafx.h"

#include "SerializeTest.h"

#include "Engine/MemoryResources.h"
#include "Engine/Factory.h"
#include "Engine/PhysicsManager.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/InputManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/GameObject.h"
#include "Engine/Serializer.h"
#include "Engine/TestComp.h"


// TODO (bug) : this test leaks 264 bytes. So does GOAndCompTest (200 bytes) and
// ScriptTest (120 bytes). Maybe they're related...


void SerializeTest::Setup(EngineExport* _p_engine_export_struct) {
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_physics_manager = _p_engine_export_struct->p_engine_physics_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif
	
	go = p_factory->BuildGameObject("TestObject.json");
	SetRunning();
}

void SerializeTest::Run() {
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SIK_INFO("Saving GameObject to file \"TestObject2.json\"");
		JSON* other_json = p_resource_manager->LoadJSON("TestObject2.json");
		p_factory->SaveObjectArchetype("TestObject2.json", *go);
		SIK_INFO("Exiting test");
		SetPassed();
		return;
	}

	SetRunning();
	return;
}

void SerializeTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
