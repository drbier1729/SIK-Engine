#pragma once

//Readable way to manage test state
enum class TestState {
	PASSED = 0,
	FAILED,
	RUNNING,
	ERR 
};

/*
* Base class for Tests.
* Every test will be a new class that inherits this.
*/
class Test
{
public:
	virtual ~Test() = default;

	virtual void Setup(EngineExport* p_engine_export_struct) = 0;
	virtual void Run() = 0;
	virtual void Teardown() = 0;
	
	/*
	* Gets the current state of the test
	* Returns: TestState
	*/
	TestState GetState() const;

	void CommonSetup();
	void CommonTeardown();

protected:
	/*
	* Sets the current state of the test to passed
	* Returns: TestState
	*/
	void SetPassed();
	/*
	* Sets the current state of the test to running
	* Returns: TestState
	*/
	void SetRunning();
	/*
	* Sets the current state of the test to failed
	* Returns: TestState
	*/
	void SetFailed();
	/*
	* Sets the current state of the test to error
	* Returns: TestState
	*/
	void SetError();

private:
	TestState current_state;
};