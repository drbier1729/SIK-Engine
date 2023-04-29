#include "stdafx.h"
#include "Test.h"

#include "Engine/Mesh.h"

void Test::CommonSetup() {
    Mesh::InitDefaults();
}

void Test::CommonTeardown() {
    Mesh::FreeDefaults();
}

/*
* Gets the current state of the test
* Returns: TestState
*/
TestState Test::GetState() const{
    return current_state;
}

/*
* Sets the current state of the test to passed
* Returns: TestState
*/
void Test::SetPassed() {
    current_state = TestState::PASSED;
}

/*
* Sets the current state of the test to Running
* Returns: TestState
*/
void Test::SetRunning() {
    current_state = TestState::RUNNING;
}

/*
* Sets the current state of the test to failed
* Returns: TestState
*/
void Test::SetFailed() {
    current_state = TestState::FAILED;
}

/*
* Sets the current state of the test to error
* Returns: TestState
*/
void Test::SetError() {
    current_state = TestState::ERR;
}
