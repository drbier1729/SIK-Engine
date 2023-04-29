#pragma once

#include "Test.h"

#include "Engine/GameObject.h"

/*
* Basic test to verify we can create game objects with working behaviour components
*/

class ScriptTest : public Test {
public:
	/*
	* Sets up the ScriptTest test.
	* 
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the sanity test
	* Steps:
	* 1) Create 1 game object with behaviour component
	* 2) Checks if it got added correctly
	* 3) Multiple scripts attached to object
	* 4) Script1 moves object
	* 5) Script2 plays audio
	* 6) Script3 checks for mouse click
	* 7) Test ends on backspace
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
};