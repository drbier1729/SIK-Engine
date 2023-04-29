#include "stdafx.h"
#include "Engine/InputManager.h"
#include "InputTests.h"

/*
* Sets up the Input test.
* Initializes the input manager pointer
* Returns: TestState
*/
void InputTests::Setup(EngineExport* _p_engine_export_struct) {
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	SetRunning();
	return;
}

/*
* Runs the Input test
* Steps:
* 1) Print log message for each action map checked
* 2) If the ACTION_SELECT is pressed then exit from loop
* Returns: TestState
*/
void InputTests::Run() {
	bool running = true;

	if (p_input_manager->IsControllerButtonPressed(p_input_manager->GetPlayerOneController(), SDL_CONTROLLER_BUTTON_A))
		SIK_INFO("CONTROLLER PRESSED IN TEST");

	for (InputAction::Actions action = static_cast<InputAction::Actions>(0);
		action < InputAction::Actions::_NUM;
		action = static_cast<InputAction::Actions>((SizeT)action + 1)) {

		switch (action) {
		case InputAction::Actions::UP:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action UP was performed");
			break;
		case InputAction::Actions::DOWN:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action DOWN was performed");
			break;
		case InputAction::Actions::LEFT:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action LEFT was performed");
			break;
		case InputAction::Actions::RIGHT:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action RIGHT was performed");
			break;
		case InputAction::Actions::ACTION_1:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action 1 was performed");
			break;
		case InputAction::Actions::ACTION_2:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action 2 was performed");
			break;
		case InputAction::Actions::ACTION_3:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action 3 was performed");
			break;
		case InputAction::Actions::ACTION_4:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action 4 was performed");
			break;
		case InputAction::Actions::ACTION_L1:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action L1 was performed");
			break;
		case InputAction::Actions::ACTION_L2:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action L2 was performed");
			break;
		case InputAction::Actions::ACTION_R1:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action R1 was performed");
			break;
		case InputAction::Actions::ACTION_R2:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action R2 was performed");
			break;
		case InputAction::Actions::ACTION_START:
			if (test_actions.IsActionPressed(action))
				SIK_INFO("Action Start was performed");
			break;
		case InputAction::Actions::ACTION_SELECT:
			if (test_actions.IsActionPressed(action)) {
				SIK_INFO("Select is pressed. Exiting Test");
				SetPassed();
				return;
			}
			break;
		case InputAction::Actions::_NUM:
			//"Default" case. Will never reach here
			break;
		}
	}
	SetRunning();
	return;
}

/*
* Runs the teardown
* Returns: TestState
*/
void InputTests::Teardown() {
	SetPassed();
}
