#include "stdafx.h"
#include "Engine/InputManager.h"
#include "Engine/InputAction.h"
#include "Engine/MemoryResources.h"
#include "Engine/GUIObject.h"
#include "Engine/Panel.h"
#include "Engine/Button.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/ResourceManager.h"
#include "GUIObjectTest.h"

void Button1Action() {
	SIK_INFO("BUTTON1 action performed");
}

void Button2Action() {
	SIK_INFO("BUTTON2 action performed");
}

void Button3Action() {
	SIK_INFO("BUTTON3 action performed");
}

/*
* Sets up the GUI object test.
* Initializes the gui_obj_manager pointer
* 1) Create 2 panel objects
* 2) Create 1 button object embedded in panel 1
* 3) Create 2 button objects embedded in panel 2
* Returns: void
*/
void GUIObjectTest::Setup(EngineExport* _p_engine_export_struct) {
	p_gui_object_manager = _p_engine_export_struct->p_engine_gui_obj_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

	prev_highlighted = curr_highlighted = -1;
	if (p_gui_object_manager) {
		p_gui_object_manager->CreateGUIFromFile("test_gui.json");
		SetRunning();
		return;
	}
	else {
		SIK_ERROR("Failed to initialize game object manager");
		SetError();
		return;
	}
}

/*
* Runs the test
* Steps:
* 1) Verify that highlighting works correctly
* 2) Verify that button action works correctly
* 3) Call update on all gui objects
* 4) Delete all gui objects
* 5) Verify size of memory resource is 0
* 6) Call update on all game objects
* Returns: void
*/
void GUIObjectTest::Run() {
	prev_highlighted = curr_highlighted;
	curr_highlighted = p_gui_object_manager->GetHighlightIndex();
	if (curr_highlighted != prev_highlighted)
		SIK_INFO("Highlight changed to : {}", curr_highlighted);

	if (test_actions.IsActionPressed(InputAction::Actions::ACTION_SELECT)) {
		SIK_INFO("Select is pressed. Exiting Test");
		SetPassed();
		return;
	}
	SetRunning();
	return;
}

void GUIObjectTest::Teardown() {
	p_gui_object_manager->DeleteAllGUIObjects();
	SetPassed();
}
