#include "stdafx.h"
#include "MemoryResources.h"
#include "GameObjectManager.h"

/*
* Create a game object using the given name and stores it
* in the list of game objects.
* Returns: pointer to GameObject
*/
GameObject* GameObjectManager::CreateGameObject(const char* obj_name) {
	return game_object_pool.emplace(obj_name);
}

/*
* Calls the Update() function for each game object
* Returns: void
*/
void GameObjectManager::Update(Float32 dt) {
	for (auto r = game_object_pool.all(); not r.is_empty(); r.pop_front()) {
		r.front().Update(dt);
	}
}

/*
* Calls the Update() function for each game object
* Returns: void
*/
void GameObjectManager::FixedUpdate(Float32 fixed_dt) {
	for (auto r = game_object_pool.all(); not r.is_empty(); r.pop_front()) {
		r.front().FixedUpdate(fixed_dt);
	}
}

/*
* Delete all game objects
* Clears the memory resources used
* Returns: void
*/
void GameObjectManager::DeleteAllGameObjects() {
	objects_to_delete.clear();
	game_object_pool.clear();
}

Bool GameObjectManager::DeleteGameObject(GameObject* _p_delete_obj) {
	_p_delete_obj->Disable();
	objects_to_delete.push_back(_p_delete_obj);
	return true;
}
/*
* Erases the game objects in the to_delete list
* Called once per frame at the end
*/
void GameObjectManager::CleanupDeletedObjects() {
	for (auto obj : objects_to_delete) {
		game_object_pool.erase(obj);
	}
	objects_to_delete.clear();
}
