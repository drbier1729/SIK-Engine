#pragma once
#include "Engine/InputAction.h"
#include "Test.h"

class InputTests : public Test {

	virtual ~InputTests() = default;
	/*
	* Sets up the Input test.
	* Initializes the input manager pointer
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the Input test
	* "Hijacks" the loop control from EngineMain
	* Steps:
	* 1) Print log message for each action map checked
	* 2) If the ACTION_SELECT is pressed then exit from loop
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Returns: void
	*/
	void Teardown() override;

private:
	InputAction test_actions{"default"};
};

