#pragma once
#include "Destroyable.h"
#include "Inventory.h"
#include "Collectable.h"
#include "PlayerController.h"

class GameObject;
struct ParticleEmitter;

class PrototypeBuilding
{
public:
	PrototypeBuilding();
	~PrototypeBuilding();
	/*
	* Adds the destroyable components to the game objects
	* Also sets the player_go pointer
	* Returns: void
	*/
	void SetupGameObjects();

	/*
	* Removes the destroyable components from the game objects
	*/
	void TeardownGameObjects();
	void HandleDestroyedObjects();

	/*
	* Creates a collectable game object at the specified location
	* Returns: void
	*/
	void SpawnCollectable(Vec3 spawn_pos);

	/*
	* Update logic for prototype
	*/
	void Update(Float32 dt);
private:
	GameObject* player_go;
	RenderCam camera;
	UniquePtr<InputAction> prototype_action_map;
	ParticleEmitter* player_particle_emitter;

	void UpdatePlayerParticles();
};

