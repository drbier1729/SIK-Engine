#pragma once
#include "Test.h"

class LocalLightsTest : public Test {
public:
	/*
	* Sets up the Local Lights test.
	* 1) Create the floor
	* 2) Create 100 local lights
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the Local Lights test
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes game object
	* Deletes local lights
	* Returns: void
	*/
	void Teardown() override;

private:
	GameObject* p_obj;
};

