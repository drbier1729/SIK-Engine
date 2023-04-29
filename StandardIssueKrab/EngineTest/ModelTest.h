#pragma once

#include "Test.h"

#include "Engine/GameObject.h"

/*
* Basic test to verify we can create game objects with model
*/

class ModelTest : public Test {
public:
	/*
	* Sets up the ModelTest test.
	*
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the sanity test
	* Steps:
	* 1) Create 1 game object with model
	* 2) Checks if it got added correctly
	* 3) Test ends on backspace
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