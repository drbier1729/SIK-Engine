#include "stdafx.h"

#include "Engine/GameInterface.h"
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
#include "Engine/GameStateManager.h"
#include "Engine/GameManager.h"

#include "BaseState.h"
#include "StartState.h"
#include "GamePlayState.h"

#include "CarController.h"
#include "FollowCam.h"
#include "Destroyable.h"
#include "PlayerCharacter.h"
#include "TurretEnemy.h"
#include "BallnChain.h"
#include "Health.h"
#include "Attachment.h"
#include "ObjectHolder.h"
#include "Collectable.h"
#include "Inventory.h"
#include "GenericCarEnemy.h"
#include "CraneEnemy.h"
#include "Debris.h"
#include "Magnet.h"
#include "Anchored.h"
#include "MetalBox.h"
#include "GarageTransport.h"
#include "DebugPhysicsInfo.h"
#include "Ocean.h"

// Extern Globals
BaseState* p_base_state = nullptr;
StartState* p_start_state = nullptr;

// TU Globals
static GUIObject* p_cheats_hud = nullptr;
static Bool cheats_active = false;
static SDL_Cursor* p_cursor = nullptr;


#define GAME_INTERFACE extern "C" __declspec(dllexport)


// Helpers
static void HandleCheatCodes();
static void RegisterGameComponents();

/*
* The launcher for the game .
* Args : (p_engine_export_struct)
* Launches game and passes in a pointer to the EngineExport struct
* Returns: void
*/
GAME_INTERFACE GAME_LAUNCH(game_launch) {
	SIK_INFO("Launching Game");
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
	p_gamestate_manager = p_engine_export_struct->p_gamestate_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
#endif

	SDL_Surface* cursor_surf = p_resource_manager->LoadImageAsSurface("cursor.png");
	if (cursor_surf) {
		p_cursor = SDL_CreateColorCursor(cursor_surf, 1, 1);
		SDL_SetCursor(p_cursor);
	}

	// factory needs to have Builders for the game side components
	RegisterGameComponents();

	// set up global cheats menu
	auto cheats_gui = p_gui_object_manager->CreateGUIFromFile("CheatsHUD.json");
	if (not cheats_gui.empty()) {
		p_cheats_hud = cheats_gui[0];
		p_cheats_hud->DisableRender();
	}
	else {
		SIK_ERROR("Cheats menu not created");
	}

	// setup game
	p_base_state = p_memory_manager->GetCurrentAllocator().new_object<BaseState>();
	p_gamestate_manager->PushState(p_base_state);

	p_start_state = p_memory_manager->GetDefaultAllocator().new_object<StartState>("start_menu.json");
	p_gamestate_manager->PushState(p_start_state);
}

/*
* The game update function
*/
GAME_INTERFACE GAME_UPDATE(game_update) {
	HandleCheatCodes();
}

/*
* The game destroy function
*/
GAME_INTERFACE GAME_END(game_end) {
	SIK_INFO("Ending Game");

	p_memory_manager->GetCurrentAllocator().delete_object(p_base_state);
	p_memory_manager->GetCurrentAllocator().delete_object(p_start_state);

	p_gamestate_manager->Clear();
	p_gui_object_manager->DeleteAllGUIObjects();
	p_game_obj_manager->DeleteAllGameObjects();
	p_scripting_manager->DeleteAllBehaviours();
	p_physics_manager->Clear();
	p_graphics_manager->Clear();
	p_audio_manager->StopAll();

	SDL_FreeCursor(p_cursor);
}


static void RegisterGameComponents() {
	p_factory->RegisterComponent<CarController>();
	p_factory->RegisterComponent<FollowCam>();
	p_factory->RegisterComponent<Destroyable>();
	p_factory->RegisterComponent<PlayerCharacter>();
	p_factory->RegisterComponent<TurretEnemy>();
	p_factory->RegisterComponent<BallnChain>();
	p_factory->RegisterComponent<Health>();
	p_factory->RegisterComponent<Attachment>();
	p_factory->RegisterComponent<ObjectHolder>();
	p_factory->RegisterComponent<GenericCarEnemy>();
	p_factory->RegisterComponent<CraneEnemy>();
	p_factory->RegisterComponent<Debris>();
	p_factory->RegisterComponent<Magnet>();
	p_factory->RegisterComponent<Anchored>();
	p_factory->RegisterComponent<Collectable>();
	p_factory->RegisterComponent<Inventory>();
	p_factory->RegisterComponent<MetalBox>();
	p_factory->RegisterComponent<GarageTransport>();
	p_factory->RegisterComponent<DebugPhysicsInfo>();
	p_factory->RegisterComponent<Ocean>();
}


static void HandleCheatCodes()
{
	if (cheats_active) {
		if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_1)) {			// Player Invincible
			if (GameObject* player = p_base_state->GetPlayerGameObjPtr(); player) {
				if (Health* health = player->HasComponent<Health>(); health) {
					health->ToggleInvincibility();
				}
			}
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_2)) {			// Kill Player
			if (GameObject* player = p_base_state->GetPlayerGameObjPtr(); player) {
				if (PlayerCharacter* pc = player->HasComponent<PlayerCharacter>(); pc) {
					Int32 max_hp = player->HasComponent<Health>()->GetMaxHP();
					pc->TakeDamage(max_hp);
				}
			}
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_3)) {			// Skip to Game End
			// TODO(Dylan): Display pop-up confirmation of destructive action

			// ...
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_4)) {			// Skip to Game Beginning

			// TODO(Dylan): Display pop-up confirmation of destructive action
			
			// clear all game data
			game_end();

			// set up global cheats menu
			auto cheats_gui = p_gui_object_manager->CreateGUIFromFile("CheatsHUD.json");
			if (not cheats_gui.empty()) {
				p_cheats_hud = cheats_gui[0];
				if (cheats_active) { p_cheats_hud->EnableRender(); }
				else			   { p_cheats_hud->DisableRender(); }
			}
			else {
				SIK_ERROR("Cheats menu not created");
			}
			
			// setup the game from scratch
			p_base_state = p_memory_manager->GetCurrentAllocator().new_object<BaseState>();
			p_gamestate_manager->PushState(p_base_state);

			p_start_state = p_memory_manager->GetDefaultAllocator().new_object<StartState>("start_menu.json");
			p_gamestate_manager->PushState(p_start_state);
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_5)) {			// No Collisions
			p_physics_manager->ToggleCollisions();
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_6)) {			// Physics Debug Drawing
			p_graphics_manager->ToggleDebugDrawing();
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_7)) {			// Get $$$$$
			if (GameObject* player = p_base_state->GetPlayerGameObjPtr(); player) {
				if (Inventory* inv = player->HasComponent<Inventory>(); inv) {
					inv->AddCollectable(Collectable::CTypes::Resource1, 10000);
				}
			}
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_8)) {			// Clear level
			if (GamePlayState* state = p_base_state->GetGamePlayState(); state) {
				state->SetProgress(Progress::Complete);
			}
		}
		else if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_9)) {			// Enter Garage
			if (GamePlayState* state = p_base_state->GetGamePlayState(); state) {
				state->EnterGarage();
			}
		}
		// Use Delete key to force quit
		if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_DELETE)) {
			SIK_INFO("Delete key pressed!");
			p_game_manager->Quit();
			SIK_INFO("Quiting...");
		}
	}

	// Activate Cheats
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_GRAVE)) {
		cheats_active = !cheats_active;
		if (p_cheats_hud) {
			if (cheats_active) { p_cheats_hud->EnableRender(); }
			else			   { p_cheats_hud->DisableRender(); }
		}
	}
}