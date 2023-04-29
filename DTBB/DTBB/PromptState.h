#pragma once

#include "Engine/GameState.h"

class GUIObject;
class GamePlayState;

class PromptState : public GameState {
public:
	PromptState();
	~PromptState();
	 
	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;
	void FixedUpdate(Float32 fixed_timestep) override;
	void HandleEvent(SDL_Event const& e) override;

	/*
	* Functions for script to access the car stats
	*/
	Float32 GetCarAmountMoved();
	Float32 GetCarAmountDrifted();
	Float32 GetDistanceToTurret();

	/*Set the junkyard state*/
	void SetJunkyardState(GamePlayState* _p_junkyard_state);
private:
	Vector<GUIObject*> dialog_objects;
	GamePlayState* p_junkyard_state;

	Float32 car_amount_moved;
	Float32 car_amount_drifted;
	Vec3 tutorial_turret_pos;

	Bool scripts_registered;

	void DialogUpdate(Float32 dt);

	/*
	* Registers the player object to the scripts.
	* Returns: void
	*/
	void RegisterPlayerToScripts();

	/*
	* Updates the car moved and drifted stats
	* Returns: void
	*/
	void UpdateCarStats(Float32 dt);

	Vec3 GetTurretPosition();
};

