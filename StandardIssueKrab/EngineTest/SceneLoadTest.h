#pragma once

#include "Test.h"

/*
* Basic test to verify we can build scenes from JSON files
*/

class SceneLoadTest : public Test {
public:
	/*
	* Sets up the SceneLoadTest test.
	*
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the sanity test
	* Steps:
	* 1) Create 3 game objects from a JSON file
	* 2) Checks if they got added correctly
	* 3) Test ends on backspace
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes game objects
	* Returns: void
	*/
	void Teardown() override;
};