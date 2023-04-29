#pragma once
#include "Test.h"
#include "Engine/InputAction.h"

class CharacterControllerTest : public Test {
public:
	/*
	* Sets up the Character controller test.
	* 1) Create a game object that can be controlled with an action map
	* 2) Disable camera control
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the CharacterControllerTest test
	* Steps:
	* 1) Update the game object
	* 2) Check for inputs
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes game object
	* Returns: void
	*/
	void Teardown() override;

private:
	GameObject* p_obj;
	InputAction* action_map;
};

