#pragma once

#include "FixedObjectPool.h"
#include "GameObject.h"

class GameObjectManager {
	static constexpr SizeT MAX_GAME_OBJECTS = 8192;
	template<class T> using Pool = FixedObjectPool<T, MAX_GAME_OBJECTS>;

private:
	Pool<GameObject> game_object_pool;
	Vector<GameObject*> objects_to_delete;

public:
	// Defaulted ctors and dtor

	/*
	* Adds a game object to the list of game objects
	* Returns: void
	*/
	GameObject* CreateGameObject(const char* obj_name = "<NO NAME>");

	/*
	* Calls the Update() function for each game object
	* Returns: void
	*/
	void Update(Float32 dt);

	/*
	* Calls the FixedUpdate() function for each game object
	* Returns: void
	*/
	void FixedUpdate(Float32 fixed_dt);

	/*
	* Clears the container holding all game objects
	* Returns: void
	*/
	void DeleteAllGameObjects();

	/*
	* Deletes a specific game object
	* Returns: Bool - True if success
	*/
	Bool DeleteGameObject(GameObject* _p_delete_obj);

	/*
	* Erases the game objects in the to_delete list
	* Called once per frame at the end
	*/
	void CleanupDeletedObjects();
	
	/*
	* Iterates the container holding all game objects and calls
	* fn on each object.
	*/
	template<class F> void ForEach(F fn);

	/*
	* Returns underlying data structure holding all game objects
	*/
	inline Pool<GameObject>&		GetGameObjectContainer() { return game_object_pool; }
	inline Pool<GameObject> const& GetGameObjectContainer() const { return game_object_pool; }
};

//Extern global variable defined so it can be accessed throughout the project
extern GameObjectManager* p_game_obj_manager;


// Inline definitions
template<class F>
void GameObjectManager::ForEach(F fn) {
	for (auto r = game_object_pool.all(); not r.is_empty(); r.pop_front()) {
		fn( r.front() );
	}
}