#pragma once
#include "Engine/GameState.h"

// forward declarations
class RenderCam;
class GameObject;
class GUIObject;
class VersusState;
class StartState;
class FadeState;
class MenuState;

enum class Progress : Int32 {
	Lose = -1,
	Start = 0,
	Working = 1,
	Complete = 2,
};

inline constexpr auto operator<=>(Progress const& a, Progress const& b) {
	return static_cast<Int32>(a) <=> static_cast<Int32>(b);
}

class GamePlayState : public GameState {
public:
	GamePlayState(const char* scene_json, StringID background_music, Bool _can_lose = true);
	~GamePlayState();

	/// <summary>
	/// Add required components to game objects.
	/// Set pointer to player game object.
	/// </summary>
	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;
	void FixedUpdate(Float32 fixed_timestep) override;

	/*
	* Handles minimize/lost focus on window events
	* by pausing and muting the game
	* Returns: void
	*/
	void HandleEvent(SDL_Event const& e) override;

 /*
  * Disables all GameObjects in this state.
	*/
	void DisableAllGameObjects();


	/*
	* Function to increment the enemy count by 1
	* Returns: void
	*/
	void IncrementTurretCount();
	/*
	* Function to decrement the enemy count by 1
	* Returns: void
	*/
	void DecrementTurretCount();
	/*
	* Get the current enemy count
	* Returns: Int8
	*/
	Int32 GetTurretCount() const;


	void IncrementSmallCarCount();
	void DecrementSmallCarCount();
	Int32 GetSmallCarCount() const;

	void IncrementBigCarCount();
	void DecrementBigCarCount();
	Int32 GetBigCarCount() const;


	/*
	* Function to increment the desctructible count by 1
	* Returns: void
	*/
	void IncrementDestructibleCount();
	/*
	* Function to decrement the desctructible count by 1
	* Returns: void
	*/
	void DecrementDestructibleCount();
	/*
	* Get the current destructible count
	* Returns: Int8
	*/
	Int32 GetDestructibleCount() const;

	/*
	* Enter Garage state
	* Returns: void
	*/
	void EnterGarage(Bool with_fade=true);

	/*
	* Enter the Pause state
	* Returns: void
	*/
	void PauseGame();

	/*
	* Gets a pointer to the fade state
	* Returns: FadeState*
	*/
	FadeState* GetFadeState();

	void EnableHUDObjects();

	/*
	* Resets the level
	* Returns: void
	*/
	void ResetLevel();

	void SetLastLevel(Bool _is_last_level);
	Bool IsLastLevel();

	void SetProgress(Progress new_progress);
	Progress GetProgress() const;
	Progress GetPreviousProgress() const;

private:
	RenderCam* camera;
	VersusState* versus_state;
	Vector<GUIObject*> hud_objects;
	Vector<GameObject*> deleted_objects;

	FadeState* p_fade_out;
	FadeState* p_fade_in;
	FadeState* p_lose_fade;
	MenuState* p_pause_state;

	Int32 turret_counter, destructible_counter, small_car_counter, big_car_counter;

	StringID music_tag;
	Int32 music_id;

	Bool start_menu_shown;
	Bool from_pause;
	Bool scripts_registered;
	Bool can_lose;
	Bool is_last_level;
	Bool fading;

	Progress current_progress;
	Progress prev_progress;

	void HUDUpdate(Float32 dt);
	/*
	* Makes sure that the shadows will always work around the player character
	* Returns: void
	*/
	void ShadowFollowPlayer();

	/*
	* Registers the player object to all the scripts in the game
	* Returns: void
	*/
	void RegisterPlayerToScripts();

	/*
	* Checks to see if we can enter the garage
	* Checks specific area of exit and handles button accordingly.
	* Returns: void
	*/
	void CheckGarageEnter();

	/*
	* Set the condition according to various factors.
	* Returns: void
	*/
	void UpdateGameCondition();

	/*
	* Handle the removed objects
	*/
	void HandleRemovedObjects();

	void ResetEnemyCountForHUD();
};

