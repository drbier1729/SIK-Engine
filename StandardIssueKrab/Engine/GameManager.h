#pragma once
#ifdef _PROTOTYPE
#include "EngineTest/Test.h"
#include "EngineTest/TestLauncher.h"
#include "PrototypeInterface.h"
#else
#include "GameInterface.h"
#endif // _PROTOTYPE

class RenderCam;

/*
* The GameManager class is used to manage the game at the highest level.
* Maintains its run status, current level, debug mode etc.
*/
class GameManager {
private:
	Bool is_running;
	Int32 level;
	std::filesystem::path exe_path;

#ifndef _PROTOTYPE
	void* game_exe_handle;
	game_launch_fcn* LaunchGame;
	game_update_fcn* UpdateGame;
	game_end_fcn* EndGame;
#else // !_PROTOTYPE
	Vector<String> TestNames;
	Vector<String> prototype_names;

	void* test_dll_handle;
	std::unique_ptr<Test> p_test_obj;
	test_launch_fcn* LaunchTest;
	test_update_fcn* UpdateTest;
	test_end_fcn* EndTest;

	Vector<void*> prototype_dll_handles;
	Vector<prototype_launch_fcn*> LaunchPrototypes;
	Vector<prototype_update_fcn*> UpdatePrototypes;
	Vector<prototype_end_fcn*> EndPrototypes;
	Int8 active_prototype_indx = -1;
#endif // _PROTOTYPE

	EngineExport engine_export_struct;
	void InitEngineExportStruct();

	Int8 run_args_count;
	char** run_args;

	RenderCam* p_engine_cam;

#ifndef _PROTOTYPE
	/*
	* Load the Game exe. Loads the exe object for the game and the
	* requried functions.
	* Returns: Bool - True if loading succeeded
	*/
	Bool LoadGameEXE();

	/*
	* Unload the Game exe. Unloads the exe object for the game.
	* Returns: Bool - True if unloading succeeded
	*/
	Bool UnloadGameEXE();

#else // !_PROTOTYPE
	/*
		* Load the Test Dll. Loads the DLL object for the test dll and the
		* requried functions.
		* Returns: Bool - True if loading succeeded
		*/
	Bool LoadTestDLL();


	/*
	* Load the Prototype Dll. Loads the DLL object for the specified dll and the
	* requried functions.
	* Returns: Bool - True if loading succeeded
	*/
	Bool LoadPrototypeDLL(Uint8 prototype_indx);

	/*
	* Unload all Prototype Dlls.
	* Returns: Bool - True if loading succeeded
	*/
	Bool UnloadPrototypeDLLs();
#endif // _PROTOTYPE

public:
	//Default Constructor
	GameManager();

	//Sets the run status to false
	void Quit();

	//Returns the current run status
	Bool Status();

	//Returns the current level
	Int32 GetLevel();

	//Sets the current level
	void SetLevel(Int32 level);

	//Update
	void Update(Float32 dt);

	/*
	* Load DLLs on startup and intialize the engine export struct
	* Args:
	* prototypes(bool) - Specified if the DLLs need to be loaded for the prototypes 
						 or the main game.
	* Returns: Bool - True if Initialization succeeded
	*/
	Bool LoadDlls();

	void UnloadDlls();

	/*
	* Sets the runtime arguments received from Main()
	* Returns: void
	*/
	void SetRunArgs(Int8 argc, char* args[]);

	/*
	* Sets the camera for the engine in editor mode
	* Returns void
	*/
	void SetEngineCam(RenderCam* p_cam);

	/*
	* Returns the default camera set during engine init
	* Returns : Pointer to render cam
	*/
	RenderCam* GetEngineCam() const;

#ifndef _PROTOTYPE
	/* Starts the game by calling the LaunchGame interface exposed by the game .exe
	* Returns: void
	*/
	void RunGame();
	void StopGame();
#else
	/*
	* Calls the LaunchPrototype() function and sets the active prototype
	* returns: Bool - True if launching suceeded
	*/
	Bool LaunchPrototype(Uint8 prototype_indx);

	/*
	* Ends the active prototype by called the EndPrototype() function
	*/
	void EndActivePrototype();

	Bool LaunchTestFromImGui(const Int8 num);

	//Launches a predefined default test
	Bool LaunchDefaultTest();

	/*
	* Checks if a test is currently running
	* Returns: Bool - True if currently running
	*/
	Bool IsTestRunning();

	/*
	* Ends the currently running test.
	* Does nothing if no test is running.
	* Returns: void
	*/
	void EndRunningTest();

	Vector<String> getTestNames() { return TestNames; }

	/*
	* Function to check if a prototype is currently running
	* Returns: Bool - True if running
	*/
	Bool IsPrototypeRunning();

	const Vector<String>& getPrototypeNames();
#endif // _PROTOTYPE
};

//Declared as an extern variable so it can be accessed throughout the project
extern GameManager* p_game_manager;
