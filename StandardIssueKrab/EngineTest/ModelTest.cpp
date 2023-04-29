#include "stdafx.h"

#include "ModelTest.h"

#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/PhysicsManager.h"

// TODO (bug) : this test leaks 120 bytes. So does SerializeTest (264 bytes) and
// GOAndCompTest (200 bytes). Maybe they're related...

void ModelTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_physics_manager = _p_engine_export_struct->p_engine_physics_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif
	if (p_factory) {
		SetRunning();
		p_obj = nullptr;
		return;
	}
	else {
		SIK_ERROR("Failed to initialize game object manager");
		SetError();
		return;
	}
}

void ModelTest::Run() {
	if (p_obj == nullptr) {
		SIK_INFO("Creating game object");
		p_obj = p_factory->BuildGameObject("ModelObject.json");

		SIK_INFO("Checking for Renderer Component");
		if (p_obj->HasComponent<MeshRenderer>() != nullptr) {
			SIK_INFO("Found");
		}
		else {
			SIK_INFO("Not found");
			SetFailed();
			return;
		}
	}

	SetRunning();
	return;
}

void ModelTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
