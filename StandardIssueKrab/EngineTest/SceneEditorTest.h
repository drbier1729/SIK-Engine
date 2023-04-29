#pragma once

#include "Test.h"

#include "Engine/GameObject.h"

/*
* Basic test to verify we can create game objects with working behaviour components
*/

class SceneEditorTest : public Test {
public:
	/*
	* Sets up the SceneEditorTest test.
	*
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the SceneEditorTest test
	* Steps:
	* 1) 
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