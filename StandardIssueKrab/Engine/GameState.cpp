#include "stdafx.h"
#include "GameObject.h"
#include "GameState.h"
#include "GameObjectManager.h"
#include "Factory.h"

GameState::GameState(const char* scene_json)
	: GameState{}
{
	objects_in_current_state = p_factory->BuildScene(scene_json);
}

GameState::~GameState() {
	for (GameObject* obj : objects_in_current_state) {
		p_game_obj_manager->DeleteGameObject(obj);
	}
}

void GameState::Enter() {
}

void GameState::Exit() {
}

void GameState::Update(Float32 dt) {	

	// Remove old objects -- this is awkward and requires
	// both arrays to be sorted. Luckily this doesn't happen often.
	if (not removed_objects.empty()) {
		std::sort(removed_objects.begin(), removed_objects.end());
		std::sort(objects_in_current_state.begin(), objects_in_current_state.end());

		auto r_it = removed_objects.begin();
		std::erase_if(objects_in_current_state,
			[this, &r_it](GameObject* x) -> Bool {
				while (r_it != removed_objects.end() && *r_it < x) { ++r_it; }
				return (r_it != removed_objects.end() && *r_it == x);
			}
		);
		
		//Delete the removed game objects
		for (GameObject* obj : removed_objects) {
			p_game_obj_manager->DeleteGameObject(obj);
		}
		removed_objects.clear();
	}
	
	// Add new objects
	for (GameObject* n : added_objects) {
		objects_in_current_state.push_back(n);
	}
	added_objects.clear();

	// Update all objects
	for (GameObject* object : objects_in_current_state) {
		object->Update(dt);
	}
}

void GameState::FixedUpdate(Float32 fixed_timestep) {
	for (GameObject* object : objects_in_current_state) {
		object->FixedUpdate(fixed_timestep);
	}
}

void GameState::HandleEvent(SDL_Event const& e)
{

}

void GameState::EnableAllObjects() {
	for (GameObject* obj : objects_in_current_state) {
		obj->Enable();
	}
}

void GameState::DisableAllObjects() {
	for (GameObject* obj : objects_in_current_state) {
		obj->Disable();
	}
}

void GameState::ResetAllObjects() {
	for (GameObject* obj : objects_in_current_state) {
		obj->Reset();
	}
}

void GameState::ResetAndEnableAllObjects() {
	for (GameObject* obj : objects_in_current_state) {
		obj->Reset();
		obj->Enable();
	}
}

void GameState::AddObject(GameObject* go) {
	added_objects.push_back(go);
}

void GameState::RemoveObject(GameObject* go) {
	removed_objects.push_back(go);
}

void GameState::RecreateScene(const char* scene_json) {
	for (GameObject* obj : objects_in_current_state) {
		p_game_obj_manager->DeleteGameObject(obj);
	}

	objects_in_current_state.clear();
	objects_in_current_state = p_factory->BuildScene(scene_json);
}

Vector<GameObject*> const& GameState::GetObjectsInState() {
	return objects_in_current_state;
}
