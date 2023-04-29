#include "stdafx.h"
#include "Engine/PrototypeInterface.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/Factory.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/InputManager.h"
#include "Engine/AudioManager.h"
#include "Engine/RenderCam.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/PhysicsManager.h"
#include "Engine/ParticleSystem.h"
#include "Engine/WorldEditor.h"

#include "PrototypeBuilding.h"

PrototypeBuilding* p_prototype;

#define PROTOTYPE_INTERFACE extern "C" __declspec(dllexport)

/*
* The launcher for the building game prototype .
* Args : (p_engine_export_struct)
* Launches building game prototype and passes in a pointer to the EngineExport struct
* Returns: void
*/
PROTOTYPE_INTERFACE PROTOTYPE_LAUNCH(prototype_launch) {
	SIK_INFO("Launching Building Prototype");
	p_resource_manager = p_engine_export_struct->p_engine_resource_manager;
	p_factory = p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = p_engine_export_struct->p_engine_game_obj_manager;
	p_graphics_manager = p_engine_export_struct->p_engine_graphics_manager;
	p_scripting_manager = p_engine_export_struct->p_engine_scripting_manager;
	p_input_manager = p_engine_export_struct->p_engine_input_manager;
	p_audio_manager = p_engine_export_struct->p_engine_audio_manager;
	p_memory_manager = p_engine_export_struct->p_engine_memory_manager;
	p_gui_object_manager = p_engine_export_struct->p_engine_gui_obj_manager;
	p_physics_manager = p_engine_export_struct->p_engine_physics_manager;
	p_particle_system = p_engine_export_struct->p_engine_particle_system;
	p_world_editor = p_engine_export_struct->p_engine_world_editor;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
#endif
	Mesh::InitDefaults();
	
	//Configure the graphics settings for the Building Prototype 
	p_graphics_manager->deferred_shading_enabled = true;
	p_graphics_manager->gamma = 2.8f;

	p_factory->RegisterComponent<Destroyable>();
	p_factory->RegisterComponent<Inventory>();
	p_factory->RegisterComponent<Collectable>();
	p_factory->RegisterComponent<PlayerController>();


	//Load the scene for the Building Prototype
	p_factory->BuildScene("PrototypeBuildingScene.json");

	//Load the GUI for the prototype
	p_gui_object_manager->CreateGUIFromFile("test_gui_prototype_building.json");

	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();

	p_prototype = alloc.new_object<PrototypeBuilding>();
	p_prototype->SetupGameObjects();
}

/*
* The prototype update function
*/
PROTOTYPE_INTERFACE PROTOTYPE_UPDATE(prototype_update) {
	p_prototype->Update(dt);
	p_prototype->HandleDestroyedObjects();
}

/*
* The prototype destroy function
*/
PROTOTYPE_INTERFACE PROTOTYPE_END(prototype_end) {
	SIK_INFO("Ending Prototype");
	PolymorphicAllocator alloc = p_memory_manager->GetDefaultAllocator();

	alloc.delete_object(p_prototype);

	p_gui_object_manager->DeleteAllGUIObjects();
	p_prototype->TeardownGameObjects();

	p_particle_system->Clear();
	p_game_obj_manager->DeleteAllGameObjects();
	p_scripting_manager->DeleteAllBehaviours();
	p_physics_manager->Clear();
	p_graphics_manager->Clear();
	
	Mesh::FreeDefaults();
}