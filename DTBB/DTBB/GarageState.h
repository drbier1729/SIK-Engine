#pragma once
#include "Engine\GameState.h"

class RenderCam;
class GUIObject;
class GamePlayState;
class FadeState;
class PromptState;

class GarageState : public GameState {
public:
	GarageState();
	~GarageState();

	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;
	void FixedUpdate(Float32 fixed_timestep) override;

	Bool HasWreckingBallUpgrade() const;
	Bool HasMagnetUpgrade() const;

	void UpgradeWreckingBall();
	void UpgradeMagnet();

	Bool CompletedJunkyard() const;
	Bool CompletedConstruction() const;
	Bool CompletedMuscleBeach() const;

	/*
	* Functions that start the fade in and set the gameplay state
	*/
	void EnterConstructionArena();
	void EnterJunkyardArena();
	void EnterMuscleArena();


	void EnterTutorialArena();
private:
	RenderCam* camera;
	RenderCam* prev_camera;
	Vector<GUIObject*> hud_objects_begin;
	GamePlayState* p_construction_state;
	GamePlayState* p_musclebeach_state;
	GamePlayState* p_junkyard_state;
	GamePlayState* p_tutorial_state;
	PromptState* p_prompt_state;
	FadeState* p_fade_in;
	FadeState* p_fade_out;

	Bool first_start;
	Bool fading;

	//Upgrades unlocked
	GameObject* p_wb_obj;
	Bool wrecking_ball_upgraded;
	GameObject* p_magnet_obj;
	Bool magnet_upgraded;

	// Original scales and offsets
	Vec3 player_orig_scale_tr;
	Vec3 magnet_orig_offset;
	Vec3 magnet_orig_scale_tr;

	void HUDUpdate(Float32 dt);

	/*
	* Registers the player object to the scripts.
	* Returns: void
	*/
	void RegisterPlayerToScripts();

	Float32	rotate_time;
	/*
	* Rotates the car over time
	* Returns: void
	*/
	void RotateCar(float dt);

	/*
	* Check if the player is entering the garage after losing.
	* If so, reset the level and restore the player health.
	* 
	*/
	void CheckLoseCondition();

	/*
	* Functions that push the specifed GameplayState onto the stack
	* Returns: void
	*/
	void StartConstructionArena();
	void StartJunkyardArena();
	void StartMuscleArena();
	void StartArena();
};

