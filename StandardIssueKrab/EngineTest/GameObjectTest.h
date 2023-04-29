#pragma once

#include "Test.h"

/*
* Basic Game Object test to verify we can create and delete game objects
*/

class GameObjectTest : public Test
{
private:
	class GameObject* controlled;

public:
	/*
	* Sets up the GameObject test.
	* Initializes the game_obj_manager pointer
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the sanity test
	* Steps:
	* 1) Create 1 game object
	* 2) Check size of memory resource
	* 3) Call update on all game objects
	* 4) Create 1000 game objects
	* 5) Check size of memory resource
	* 6) Call update on all game objects
	* 7) Delete all game objects
	* 8) Verify size of memory resource is 0
	* 9) Call update on all game objects
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes all game objects
	* Returns: void
	*/
	void Teardown() override;
};