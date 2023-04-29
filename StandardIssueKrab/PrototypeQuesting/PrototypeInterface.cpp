#include "stdafx.h"

#include "Engine/PrototypeInterface.h"

#include "Engine/GameManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/Factory.h"
#include "Engine/ResourceManager.h"
#include "Engine/PhysicsManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/ParticleSystem.h"
#include "Engine/ScriptingManager.h"
#include "Engine/AudioManager.h"
#include "Engine/MemoryManager.h"
#include "Engine/WorldEditor.h"
#include "PrototypeCombat.h"
#include "Player.h"
#include "Attachment.h"
#include "Turret.h"
#include "Enemy.h"
#include "CameraFollow.h"
#include "Health.h"

PrototypeCombat* p_prototype_combat;

#define PROTOTYPE_INTERFACE extern "C" __declspec(dllexport)

/*
* The launcher for the Combat game prototype .
* Args : (p_engine_export_struct)
* Launches Combat game prototype and passes in a pointer to the EngineExport struct
* Returns: void
*/
PROTOTYPE_INTERFACE PROTOTYPE_LAUNCH(prototype_launch) {
	p_game_manager = p_engine_export_struct->p_engine_game_manager;
	p_graphics_manager = p_engine_export_struct->p_engine_graphics_manager;
	p_input_manager = p_engine_export_struct->p_engine_input_manager;
	p_game_obj_manager = p_engine_export_struct->p_engine_game_obj_manager;
	p_factory = p_engine_export_struct->p_engine_factory;
	p_resource_manager = p_engine_export_struct->p_engine_resource_manager;
	p_physics_manager = p_engine_export_struct->p_engine_physics_manager;
	p_gui_object_manager = p_engine_export_struct->p_engine_gui_obj_manager;
	p_particle_system = p_engine_export_struct->p_engine_particle_system;
	p_scripting_manager = p_engine_export_struct->p_engine_scripting_manager;
	p_audio_manager = p_engine_export_struct->p_engine_audio_manager;
	p_memory_manager = p_engine_export_struct->p_engine_memory_manager;
	p_world_editor = p_engine_export_struct->p_engine_world_editor;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
#endif

	p_prototype_combat = p_memory_manager->GetCurrentAllocator().new_object<PrototypeCombat>();

	//Configure the graphics settings for the Building Prototype 
	p_graphics_manager->deferred_shading_enabled = true;
	p_graphics_manager->gamma = 2.8f;

	p_resource_manager->InitDefaultAssets();

	// factory needs to have Builders for the game side components
	p_factory->RegisterComponent<Player>();
	p_factory->RegisterComponent<Attachment>();
	p_factory->RegisterComponent<Turret>();
	p_factory->RegisterComponent<Enemy>();
	p_factory->RegisterComponent<CameraFollow>();
	p_factory->RegisterComponent<Health>();

	// player needs to be built first as Attachment component searches by name
	p_factory->BuildGameObject("CombatPlayer.json");
	// Build scene for combat prototype
	p_factory->BuildScene("PrototypeCombat.json");

	// registers game side component functions used by scripts
	p_prototype_combat->SetupPrototype();
}

/*
* The prototype update function
*/
PROTOTYPE_INTERFACE PROTOTYPE_UPDATE(prototype_update) {
	p_prototype_combat->UpdatePrototype();
}

/*
* The prototype destroy function
*/
PROTOTYPE_INTERFACE PROTOTYPE_END(prototype_end) {
	p_memory_manager->GetCurrentAllocator().delete_object(p_prototype_combat);

	p_gui_object_manager->DeleteAllGUIObjects();
	p_game_obj_manager->DeleteAllGameObjects();
	p_scripting_manager->DeleteAllBehaviours();
	p_physics_manager->Clear();
	p_graphics_manager->Clear();
	p_resource_manager->FreeDefaultAssets();
}