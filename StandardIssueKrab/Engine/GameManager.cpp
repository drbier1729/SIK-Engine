#include "stdafx.h"
#include "MemoryResources.h"
#include "GraphicsManager.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "Factory.h"
#include "PhysicsManager.h"
#include "GUIObjectManager.h"
#include "ParticleSystem.h"
#include "ScriptingManager.h"
#include "AudioManager.h"
#include "WorldEditor.h"
#include "ImGuiWindow.h"
#include "FrameTimer.h"
#include "GameStateManager.h"
#include "GameManager.h"

#include <debugapi.h>

//Default Constructor
GameManager::GameManager() : is_running(true), level(1), 
	exe_path(std::filesystem::current_path())
#ifdef _PROTOTYPE
	, TestNames{
		"SanityTest",
		"MemoryResourcesTest",
		"GameObjectTest",
		"FixedObjectPoolTest",
		"TextureTest",
		"SerializeTest",
		"InputTests",
		"MeshTest",
		"PacketSendAndRecvTest",
		"CollisionPrimitivesTest",
		"GOandCompTest",
		"ParticlesTest",
		"ScriptTest",
		"GUITest",
		"ResourceLoadingTest",
		"ReallyStressfulTest",
		"ModelTest",
		"AudioMixingTest",
		"CharacterControllerTest",
		"SceneLoadTest",
		"LocalLightsTest",
		"SceneEditorTest"
	},
	prototype_names{
		"Driving",
		"Building",
		"Questing"
}
#endif // _PROTOTYPE
{

}

//Sets the run status to false
void GameManager::Quit() {
	is_running = false;

	p_gamestate_manager->Clear();

#ifdef _PROTOTYPE
	if (IsTestRunning())
		EndRunningTest();

	if (IsPrototypeRunning())
		EndActivePrototype();
#endif // _PROTOTYPE
}

//Returns the current run status
Bool GameManager::Status() {
	return is_running;
}

//Returns the current level
Int32 GameManager::GetLevel() {
	return level;
}

//Sets the current level
void GameManager::SetLevel(Int32 next_level) {
	level = next_level;
}

/*
* Called in the game update loop
* Checks if a test needs to be launched
* Checks if the window needs to be closed
* Returns: void
*/
void GameManager::Update(Float32 dt) {
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_F11)) {
		p_graphics_manager->ToggleFullScreen();
	}

#ifndef _PROTOTYPE
	UpdateGame(dt);

	// reload
	//if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_F5)) {
	//	StopGame();
	//	RunGame();
	//}
#else
	//If there is no test currently running
	void* p_ret_obj = nullptr;
	if (p_test_obj != nullptr) {
		//Test is currently running. Call Update on it
		p_ret_obj = UpdateTest(p_test_obj.release());
	}

	if (p_ret_obj != nullptr)
		p_test_obj.reset(static_cast<Test*>(p_ret_obj));


	if (active_prototype_indx != -1) {
		UpdatePrototypes[active_prototype_indx](dt);
	}
#endif // _PROTOTYPE
}

/*
* Function to load all the DLLs required to run the games
* Args:
* prototypes(bool) - Specified if the DLLs need to be loaded for the prototypes
*					 or the main game.
* Returns : bool - True if success
*/
bool GameManager::LoadDlls() {
	bool status = false;
#ifndef _PROTOTYPE
	status = LoadGameEXE();
#else //!_PROTOTYPE
	SIK_DEBUG("Loading DLLs for prototypes")
		status = LoadTestDLL();
	for (Uint8 i = 0; i < prototype_names.size(); ++i) {
		status &= LoadPrototypeDLL(i);
	}
#endif // _PROTOTYPE

	InitEngineExportStruct();
	return status;
}

/*
* Function to Unload all the DLLs required to run the games
* Returns : void
*/
void GameManager::UnloadDlls() {
#ifndef _PROTOTYPE
	EndGame();
	UnloadGameEXE();
#else //!_PROTOTYPE
	SDL_UnloadObject(test_dll_handle);
	UnloadPrototypeDLLs();
#endif // _PROTOTYPE
}

void GameManager::InitEngineExportStruct() {
	engine_export_struct.p_engine_game_manager = p_game_manager;
	engine_export_struct.p_engine_graphics_manager = p_graphics_manager;
	engine_export_struct.p_engine_input_manager = p_input_manager;
	engine_export_struct.p_engine_game_obj_manager = p_game_obj_manager;
	engine_export_struct.p_engine_factory = p_factory;
	engine_export_struct.p_engine_resource_manager = p_resource_manager;
	engine_export_struct.p_engine_physics_manager = p_physics_manager;
	engine_export_struct.p_engine_gui_obj_manager = p_gui_object_manager;
	engine_export_struct.p_engine_particle_system = p_particle_system;
	engine_export_struct.p_engine_scripting_manager = p_scripting_manager;
	engine_export_struct.p_engine_audio_manager = p_audio_manager;
	engine_export_struct.p_engine_memory_manager = p_memory_manager;
	engine_export_struct.p_engine_world_editor = p_world_editor;
	engine_export_struct.p_engine_imgui_window = p_imgui_window;
	engine_export_struct.p_engine_frame_timer = p_frame_timer;
	engine_export_struct.p_gamestate_manager = p_gamestate_manager;
#ifdef STR_DEBUG
	engine_export_struct.p_dbg_string_dictionary = p_dbg_string_dictionary;
#endif
}

void GameManager::SetRunArgs(Int8 argc, char* args[]) {
	run_args_count = argc;
	run_args = args;
}

void GameManager::SetEngineCam(RenderCam* p_cam) {
	p_engine_cam = p_cam;
}

RenderCam* GameManager::GetEngineCam() const {
	return p_engine_cam;
}

#ifndef _PROTOTYPE

Bool GameManager::LoadGameEXE() {
	std::filesystem::path load_path = exe_path;

	if (IsDebuggerPresent()) {
		load_path /= "x64";
#ifdef _DEBUG
		load_path /= "Debug";
#else
		load_path /= "Release";
#endif
	}
	load_path /= "DTBB.exe";

	// Loading dll object
	game_exe_handle = SDL_LoadObject(load_path.string().c_str());
	if (!game_exe_handle) {
		SIK_ERROR("Failed loading game Exe. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_launch function
	LaunchGame = (game_launch_fcn*)SDL_LoadFunction(game_exe_handle, "game_launch");
	if (!LaunchGame)
	{
		SIK_ERROR("Function LaunchGame failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_update function
	UpdateGame = (game_update_fcn*)SDL_LoadFunction(game_exe_handle, "game_update");
	if (!UpdateGame)
	{
		SIK_ERROR("Function UpdateGame failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_end function
	EndGame = (game_end_fcn*)SDL_LoadFunction(game_exe_handle, "game_end");
	if (!EndGame)
	{
		SIK_ERROR("Function EndGame failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	return true;
}

bool GameManager::UnloadGameEXE() {
	SDL_UnloadObject(game_exe_handle);
	return true;
}


void GameManager::RunGame() {
	LaunchGame(&engine_export_struct, run_args_count, run_args);
}

void GameManager::StopGame() {
	EndGame();
}
#else

bool GameManager::LoadTestDLL() {
#ifdef _DEBUG
	const char* dll = ".\\x64\\Debug\\EngineTest.dll";
#else
	const char* dll = ".\\x64\\Release\\EngineTest.dll";
#endif

	// Loading dll object
	test_dll_handle = SDL_LoadObject(dll);
	if (!test_dll_handle) {
		SIK_ERROR("EngineTest failed to load from DLL. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_launch function
	LaunchTest = (test_launch_fcn*)SDL_LoadFunction(test_dll_handle, "test_launch");
	if (!LaunchTest)
	{
		SIK_ERROR("Function TestLaunch failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_update function
	UpdateTest = (test_update_fcn*)SDL_LoadFunction(test_dll_handle, "test_update");
	if (!UpdateTest)
	{
		SIK_ERROR("Function UpdateTest failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}

	// Loading test_end function
	EndTest = (test_end_fcn*)SDL_LoadFunction(test_dll_handle, "test_end");
	if (!EndTest)
	{
		SIK_ERROR("Function EndTest failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	return true;
}

bool GameManager::LoadPrototypeDLL(Uint8 prototype_indx) {
#ifdef _DEBUG
	std::filesystem::path dll = exe_path / "x64" / "Debug";
#else
	std::filesystem::path dll = exe_path / "x64" / "Release";
#endif

	String prototype_filename = "Prototype" + prototype_names[prototype_indx] + ".dll";
	std::filesystem::path dll_path = dll / prototype_filename;

	// Loading dll object
	void* prototype_dll_handle = SDL_LoadObject(dll_path.string().c_str());
	if (!prototype_dll_handle) {
		SIK_ERROR("EngineTest failed to load from DLL. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	prototype_dll_handles.push_back(prototype_dll_handle);

	// Loading prototype launch function
	prototype_launch_fcn* LaunchPrototype = (prototype_launch_fcn*)SDL_LoadFunction(prototype_dll_handle, "prototype_launch");
	if (!LaunchPrototype)
	{
		SIK_ERROR("Function LaunchPrototype failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	LaunchPrototypes.push_back(LaunchPrototype);

	// Loading test_update function
	prototype_update_fcn* UpdatePrototype = (prototype_update_fcn*)SDL_LoadFunction(prototype_dll_handle, "prototype_update");
	if (!UpdatePrototype)
	{
		SIK_ERROR("Function UpdatePrototype failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	UpdatePrototypes.push_back(UpdatePrototype);

	// Loading test_end function
	prototype_end_fcn* EndPrototype = (prototype_end_fcn*)SDL_LoadFunction(prototype_dll_handle, "prototype_end");
	if (!EndPrototype)
	{
		SIK_ERROR("Function EndPrototype failed to load. SDL Error: \"{}\"", SDL_GetError());
		return false;
	}
	EndPrototypes.push_back(EndPrototype);

	return true;
}

bool GameManager::UnloadPrototypeDLLs() {
	EndActivePrototype();
	for (auto& prototype_dll_handle : prototype_dll_handles)
		SDL_UnloadObject(prototype_dll_handle);

	return true;
}

Bool GameManager::LaunchPrototype(Uint8 prototype_indx) {
	if (active_prototype_indx != prototype_indx) {
		active_prototype_indx = prototype_indx;
		LaunchPrototypes[active_prototype_indx](&engine_export_struct, run_args_count, run_args);
		return true;
	}
	SIK_WARN("Prototype {} already running", prototype_indx);
	return false;
}

/*
* Ends the active prototype by called the EndPrototype() function
*/
void GameManager::EndActivePrototype() {
	if (not IsPrototypeRunning())
		return;

	if (p_graphics_manager->GetPActiveCam() != p_engine_cam)
		p_graphics_manager->SetPActiveCam(p_engine_cam);

	EndPrototypes[active_prototype_indx]();
	active_prototype_indx = -1;
}

// Same as update() but launch form ImGui
//Returns: bool - True if the test is currently running.
Bool GameManager::LaunchTestFromImGui(const Int8 num) {
	//If there is no test currently running
	void* p_ret_obj = nullptr;
	if (p_test_obj == nullptr) {
		p_ret_obj = LaunchTest(num, &engine_export_struct);
	}

	if (p_ret_obj != nullptr) {
		p_test_obj.reset(static_cast<Test*>(p_ret_obj));
		return true;
	}
	else
		return false;
}

Bool GameManager::LaunchDefaultTest() {
	int default_test = 15;
	// If there is no test currently running
	void* p_ret_obj = nullptr;
	if (p_test_obj == nullptr) {
		p_ret_obj = LaunchTest(default_test, &engine_export_struct);
	}

	if (p_ret_obj != nullptr) {
		p_test_obj.reset(static_cast<Test*>(p_ret_obj));
		return true;
	}
	else
		return false;
}

/*
* Checks if a test is currently running
* Returns: Bool - True if currently running
*/
Bool GameManager::IsTestRunning() {
	if (p_test_obj != nullptr)
		return true;
	return false;
}

/*
* Ends the currently running test.
* Does nothing if no test is running.
* Returns: void
*/
void GameManager::EndRunningTest() {
	if (IsTestRunning()) {
		EndTest(p_test_obj.release());
	}
	else
		SIK_WARN("No test currently running to end.");
}

Bool GameManager::IsPrototypeRunning() {
	return (active_prototype_indx != -1);
}

const Vector<String>& GameManager::getPrototypeNames() {
	return prototype_names;
}

#endif // _PROTOTYPE
