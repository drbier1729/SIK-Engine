#pragma once

#include "Test.h"

/*
* Basic sanity test to ensure engine is working at the most basic level.
* Runs and returns quickly.
* Currently only uses some game_manager functions.
*/
//TO-DO: Make this a real sanity test.
class SanityTest : public Test
{
public:
	/*
	* Sets up the Sanity test.
	* Currently only initializes the game_manager pointer
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the sanity test
	* Steps:
	* 1) Set level to 0.
	* 2) Verify level was set to 0
	* 3) Set level to 10.
	* 4) verify level was set to 10.
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Resets the game back to level 0
	* Returns: void
	*/
	void Teardown() override;
};