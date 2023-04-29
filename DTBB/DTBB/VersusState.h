#pragma once
#include "Engine/GameState.h"

class RenderCam;
class GUIObject;

class VersusState : public GameState  {
public:
	VersusState();
	~VersusState();

	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;
	void FixedUpdate(Float32 fixed_timestep) override;

	/*
	* Gets the game object for player two
	* Returns: GameObject* - Pointer to the game object
	*/
	GameObject* GetPlayerTwoGameObjPtr() const;

	/*
	* Sets the player 2 game object 
	* Returns: void
	*/
	void SetPlayerTwoGameObjPtr(GameObject* p_obj);
private:
	RenderCam* camera;
	RenderCam* prev_camera;
	GameObject* p_player_2_go;
	Vector<GUIObject*> hud_objects;

	void HUDUpdate(Float32 dt);
	
	/*
	* Registers the player two object to the scripts.
	* Returns: void
	*/
	void RegisterPlayerToScripts();

	/*
	* Function to update the camera based on the position of player 1 and 2
	* Returns:void
	*/
	void CameraUpdate(Float32 dt);
};

extern VersusState* p_versus_state;
