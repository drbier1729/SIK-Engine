#include "stdafx.h"

#include "TestLauncher.h"
#include "SanityTest.h"
#include "MemoryResourcesTest.h"
#include "GameObjectTest.h"
#include "FixedObjectPoolTest.h"
#include "SerializeTest.h"
#include "TextureTest.h"
#include "MeshTest.h"
#include "InputTests.h"
#include "PacketSendRecvTest.h"
#include "CollisionPrimitivesTest.h"
#include "GOandCompTest.h"
#include "GUIObjectTest.h"
#include "ParticlesTest.h"
#include "ScriptTest.h"
#include "ResourceLoadingTest.h"
#include "ReallyStressfulTest.h"
#include "ModelTest.h"
#include "AudioMixingTest.h"
#include "CharacterControllerTest.h"
#include "SceneLoadTest.h"
#include "LocalLightsTest.h"
#include "SceneEditorTest.h"

#define TEST_INTERFACE extern "C" __declspec(dllexport)

/*
* The test launcher. 
* Args : (test_number , p_engine_export_struct)
* Launches the test specified by the test_number and outputs status to console.
* Returns: Test* - Returns a pointer to the test if it's still running. Else nullptr. 
*/
TEST_INTERFACE TEST_LAUNCHER(test_launch) {
	std::unique_ptr<Test> test_to_run = nullptr;
	
	switch (test_number) {
	case 0:
		test_to_run.reset(new SanityTest{});
		break;
	case 1:
		test_to_run.reset(new MemoryResourcesTest{});
		break;
	case 2:
		test_to_run.reset(new GameObjectTest{});
		break;
	case 3:
		test_to_run.reset(new FixedObjectPoolTest{});
		break;
	case 4:
		test_to_run.reset(new TextureTest{});
		break;
	case 5:
		test_to_run.reset(new SerializeTest{});
		break;
	case 6:
		test_to_run.reset(new InputTests{});
		break;
	case 7:
		test_to_run.reset(new MeshTest{});
		break;
	case 8:
		test_to_run.reset(new PacketSendRecvTest{});
		break;
	case 9:
		test_to_run.reset(new CollisionPrimitivesTest{});
		break;
	case 10:
		test_to_run.reset(new GOandCompTest{});
		break;
	case 11:
		test_to_run.reset(new ParticlesTest{});
		break;
	case 12:
		test_to_run.reset(new ScriptTest{});
		break;
	case 13:
		test_to_run.reset(new GUIObjectTest{});
		break;
	case 14:
		test_to_run.reset(new ResourceLoadingTest{});
		break;
	case 15:
		test_to_run.reset(new ReallyStressfulTest{});
		break;
	case 16:
		test_to_run.reset(new ModelTest{});
		break;
	case 17:
		test_to_run.reset(new AudioMixingTest{});
		break;
	case 18:
		test_to_run.reset(new CharacterControllerTest{});
		break;
	case 19:
		test_to_run.reset(new SceneLoadTest{});
		break;
	case 20:
		test_to_run.reset(new LocalLightsTest{});
		break;
	case 21:
		test_to_run.reset(new SceneEditorTest{});
		break;
	default: break;
	}

	if (test_to_run == nullptr) {
		SIK_ERROR("Invalid test number given: {}", test_number);
		return nullptr;
	}

	TestState curr_state;
	SIK_INFO("Runing Setup for Test \"{}\"", test_number);
	test_to_run->CommonSetup();
	test_to_run->Setup(p_engine_export_struct);
	curr_state = test_to_run->GetState();
	if (curr_state != TestState::RUNNING) {
		SIK_ERROR("Test exited in Setup with code : \"{}\"", int(curr_state));
		return nullptr;
	}

	SIK_INFO("Runing Test \"{}\"", test_number);
	test_to_run->Run();
	curr_state = test_to_run->GetState();
	if (curr_state == TestState::RUNNING) {
		return test_to_run.release();
	}
	else if (curr_state != TestState::PASSED) {
		SIK_ERROR("Test is no longer running but didn't pass. \
			Exited in Run with code : \"{}\"", int(curr_state));
	}

	SIK_INFO("Runing Teardown for Test \"{}\"", test_number);
	test_to_run->Teardown();
	test_to_run->CommonTeardown();
	curr_state = test_to_run->GetState();
	if (curr_state != TestState::PASSED) {
		SIK_ERROR("Test exited in Teardown with code : \"{}\"", int(curr_state));
	}

	SIK_INFO("Test completed successfully");
	return nullptr;
}

/*
* The test update.
* Args : (test_obj)
* Continues a test run that was previously launched.
* Returns: Test* - Returns a pointer to the test if it's still running. Else nullptr. 
*/
TEST_INTERFACE TEST_UPDATE(test_update) {
	std::unique_ptr<Test> test_to_run = nullptr;
	test_to_run.reset(static_cast<Test*>(test_obj));

	test_to_run->Run();
	TestState curr_state = test_to_run->GetState();
	if (curr_state == TestState::RUNNING) {
		return test_to_run.release();
	}
	else if (curr_state != TestState::PASSED) {
		SIK_ERROR("Test is no longer running but didn't pass. \
			Exited in Run with code : \"{}\"", int(curr_state));
	}

	SIK_INFO("Runing Teardown for Test");
	test_to_run->Teardown();
	test_to_run->CommonTeardown();
	curr_state = test_to_run->GetState();
	if (curr_state != TestState::PASSED) {
		SIK_ERROR("Test exited in Teardown with code : \"{}\"", int(curr_state));
	}

	SIK_INFO("Test completed successfully");
	return nullptr;
}

/*
* Test end function. 
* Args : (test_obj) 
* Ends the test that's passed as the argument if it's running.
* Returns: void
*/
TEST_INTERFACE TEST_END(test_end) {
	std::unique_ptr<Test> test_to_run = nullptr;
	test_to_run.reset(static_cast<Test*>(test_obj));

	SIK_INFO("Runing Teardown for Test");
	test_to_run->Teardown();
	TestState curr_state = test_to_run->GetState();
	if (curr_state != TestState::PASSED) {
		SIK_ERROR("Test exited in Teardown with code : \"{}\"", int(curr_state));
	}
}