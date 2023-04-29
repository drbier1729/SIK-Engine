#include "stdafx.h"

#include "Engine/Factory.h"
#include "Engine/GraphicsManager.h"
#include "Engine/MemoryManager.h"
#include "Engine/GameStateManager.h"
#include "Engine/GameObject.h"
#include "Engine/GameObjectManager.h"

#include "BaseState.h"
#include "GamePlayState.h"
#include "GarageState.h"

BaseState::BaseState(): 
	p_player_go{ nullptr }, p_garage_state{ nullptr }, p_gameplay_state{ nullptr } {

	
}

BaseState::~BaseState() {
	p_memory_manager->GetDefaultAllocator().delete_object(p_garage_state);
	
	for (auto& game_obj : objects_in_current_state) {
		p_game_obj_manager->DeleteGameObject(game_obj);
	}
}

void BaseState::Enter() {
	//Configure the graphics settings for the game
	p_graphics_manager->deferred_shading_enabled = true;
	p_graphics_manager->gamma = 3.6f;
	p_graphics_manager->exposure = 8.0f;
	p_graphics_manager->bloom_threshold = 0.20f;
	p_graphics_manager->SetPrimaryLightPosition(Vec3(50, 50, 50));
	
	if (objects_in_current_state.size() == 0)
		objects_in_current_state = p_factory->BuildScene("PlayerObjectScene.json");

	if (p_garage_state == nullptr) {
		p_garage_state = p_memory_manager->GetDefaultAllocator().
			new_object<GarageState>();
	}
	
	p_gamestate_manager->PushState(p_garage_state);
}

void BaseState::Exit() {
}

void BaseState::Update(Float32 dt) {
	GameState::Update(dt);
}

void BaseState::FixedUpdate(Float32 fixed_timestep) {
	GameState::FixedUpdate(fixed_timestep);
}

void BaseState::EnableAllObjects() {
	for (auto& obj : objects_in_current_state) {
		obj->Enable();
	}
}

void BaseState::DisableAllObjects() {
	for (auto& obj : objects_in_current_state) {
		obj->Disable();
	}
}

GameObject* BaseState::GetPlayerGameObjPtr() const {
	return p_player_go;
}

void BaseState::SetPlayerGameObjPtr(GameObject* _p_obj) {
	p_player_go = _p_obj;
}

GamePlayState* BaseState::GetGamePlayState() const {
	return p_gameplay_state;
}

void BaseState::SetGamePlayState(GamePlayState* _p_gameplay_state) {
	p_gameplay_state = _p_gameplay_state;
}

void BaseState::AddGameObject(GameObject* _p_obj) {
	objects_in_current_state.push_back(_p_obj);
}
