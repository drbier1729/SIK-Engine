#pragma once

#include "Test.h"

#include "Engine/GameObject.h"

/*
* Basic test to verify we can create game objects with working behaviour components
*/

class AudioMixingTest : public Test {
public:
	GameObject* controlled;

	/*
	* Sets up the AudioMixing test.
	*
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the AudioMixing test
	* Steps:
	* 1) Create 1 game object with behaviour component
	* 2) Checks if it got added correctly
	* 3) Script checks for right click
	* 4) Test ends on backspace
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
};