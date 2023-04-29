#pragma once
#include "Engine/GameState.h"

// forward declarations
class GameObject;
class GamePlayState;
class GarageState;

class BaseState : public GameState {
public:
	BaseState();
	~BaseState();

	/// <summary>
	/// Add required components to game objects.
	/// Set pointer to player game object.
	/// </summary>
	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;
	void FixedUpdate(Float32 fixed_timestep) override;

	void EnableAllObjects();
	void DisableAllObjects();

	/*
	* Gets the Player game object pointer for global access
	* Returns: GameObject* - Pointer to the game object
	*/
	GameObject* GetPlayerGameObjPtr() const;

	/*
	* Sets the Player game object pointer for global access
	* Returns: void
	*/
	void SetPlayerGameObjPtr(GameObject* _p_obj);

	/*
	* Gets the current GamePlayState if it exists
	* Returns: GamePlayState* - Ptr to gameplay state, nullptr if none
	*/
	GamePlayState* GetGamePlayState() const;

	/*
	* Sets the current GamePlayState if it exists
	* Returns: void
	*/
	void SetGamePlayState(GamePlayState* _p_gameplay_state);

	/*
	* Adds a game object to the base state.
	* Required to add car upgrades.
	* Returns: void
	*/
	void AddGameObject(GameObject* _p_obj);
private:
	GameObject* p_player_go;
	GarageState* p_garage_state;
	GamePlayState* p_gameplay_state;
};

extern BaseState* p_base_state;