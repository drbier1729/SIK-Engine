#pragma once
#include "Test.h"

class GUIObject;

class GUIObjectTest : public Test
{
private:
	GUIObject *panel_obj1, *panel_obj2, *button_obj1, *button_obj2, *button_obj3;
	InputAction test_actions{ "default" };
	Int16 prev_highlighted, curr_highlighted;
public:
	/*
	* Sets up the GUI object test.
	* Initializes the gui_obj_manager pointer
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the test
	* Steps:
	* 1) Create 2 panel objects
	* 2) Create 1 button object embedded in panel 1
	* 3) Create 2 button objects embedded in panel 2
	* 4) Verify that highlighting works correctly
	* 5) Verify that button action works correctly
	* 6) Call update on all gui objects
	* 7) Delete all gui objects
	* 8) Verify size of memory resource is 0
	* 9) Call update on all game objects
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes all gui objects
	* Returns: void
	*/
	void Teardown() override;
};

