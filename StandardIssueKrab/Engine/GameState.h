#pragma once

class GameObject;

class GameState {

public:
	GameState() = default;
	GameState(const char* scene_json);
	virtual ~GameState();
	
	virtual void Enter();
	virtual void Exit();
	virtual void Update(Float32 dt);
	virtual void FixedUpdate(Float32 fixed_timestep);
	virtual void HandleEvent(SDL_Event const& e);
	virtual void AddObject(GameObject* go);
	virtual void RemoveObject(GameObject* go);
	virtual void RecreateScene(const char* scene_json);

	Vector<GameObject*> const& GetObjectsInState();

	/*
	* Methods to enable/disable/Reset all the objects in the current state
	* Returns: void
	*/
	void EnableAllObjects();
	void DisableAllObjects();
	void ResetAllObjects();
	void ResetAndEnableAllObjects();

protected:
	Vector<GameObject*> objects_in_current_state;
	Vector<GameObject*> added_objects;
	Vector<GameObject*> removed_objects;

};