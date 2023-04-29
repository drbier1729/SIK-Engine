#include "stdafx.h"

#include "Engine/PrototypeInterface.h"
#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GameObject.h"
#include "Engine/InputAction.h"
#include "Engine/GraphicsManager.h"
#include "Engine/MemoryManager.h"
#include "Engine/PhysicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/RenderCam.h"
#include "Engine/Transform.h"
#include "Engine/GUIObject.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/ImGuiWindow.h"
#include "Engine/AudioManager.h"
#include "Engine/FrameTimer.h"
#include "Engine/WorldEditor.h"
#include "Player.h"
#include "CountdownTimer.h"

#include "Waypoint.h"
#include "ObjectiveTracker.h"
#include "DrivingCameraFollow.h"

#define PROTOTYPE_INTERFACE extern "C" __declspec(dllexport)


Player* player;
RenderCam* active_cam;
GameObject* floor_object;
CountdownTimer* countdown_timer;

GameObject* compass_object;
ObjectiveTracker* compass_objectives;

void ResetGame()
{
	player->ResetPlayer();
	countdown_timer->ResetTimer();
}

/*
* The launcher for the Driving game prototype .
* Args : (p_engine_export_struct)
* Launches Driving game prototype and passes in a pointer to the EngineExport struct
* Returns: void
*/
PROTOTYPE_INTERFACE PROTOTYPE_LAUNCH(prototype_launch) {
	SIK_INFO("Launching Driving Prototype");

	p_resource_manager = p_engine_export_struct->p_engine_resource_manager;
	p_factory = p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = p_engine_export_struct->p_engine_game_obj_manager;
	p_input_manager = p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = p_engine_export_struct->p_engine_graphics_manager;
	p_scripting_manager = p_engine_export_struct->p_engine_scripting_manager;
	p_physics_manager = p_engine_export_struct->p_engine_physics_manager;
	p_memory_manager = p_engine_export_struct->p_engine_memory_manager;
	p_gui_object_manager = p_engine_export_struct->p_engine_gui_obj_manager;
	p_imgui_window = p_engine_export_struct->p_engine_imgui_window;
	p_audio_manager = p_engine_export_struct->p_engine_audio_manager;
	p_frame_timer = p_engine_export_struct->p_engine_frame_timer;
	p_world_editor = p_engine_export_struct->p_engine_world_editor;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
#endif

	p_factory->RegisterComponent<Waypoint>();
	p_factory->RegisterComponent<ObjectiveTracker>();
	p_factory->RegisterComponent<DrivingCameraFollow>();


	p_resource_manager->InitDefaultAssets();
	p_graphics_manager->SetSkyBoxToggle();

	p_graphics_manager->deferred_shading_enabled = true;
	p_graphics_manager->gamma = 1.5;

	p_gui_object_manager->CreateGUIFromFile("Countdowntimer_gui.json");

	active_cam = p_graphics_manager->GetPActiveCam();

	active_cam->SetPosition(Vec3(10.f, 50.0f, 0.0f));
	active_cam->SetControllerMode(ControlMode::ISOMETRIC);
	active_cam->SetOrthoSize(15.0f);

	float track_height = 5.5f; // Height of -0.75f is at floor level, consider using for demo

	{
		//Build the floor 1
		floor_object = p_factory->BuildGameObject("RoadObject.json");

		////Build the floor 2
		floor_object = p_factory->BuildGameObject("RoadObject2.json");

		//Build the floor 3
		floor_object = p_factory->BuildGameObject("RoadObject3.json");

		////Build the floor 4
		floor_object = p_factory->BuildGameObject("RoadObject4.json");

		//Build the floor 5
		floor_object = p_factory->BuildGameObject("RoadObject5.json");

		////Build the floor 6
		floor_object = p_factory->BuildGameObject("RoadObject6.json");
		
		//Build the floor 7
		floor_object = p_factory->BuildGameObject("RoadObject7.json");

		////Build the floor 8
		floor_object = p_factory->BuildGameObject("RoadObject8.json");
	}

	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();

	player = alloc.new_object<Player>();
	countdown_timer = alloc.new_object<CountdownTimer>();

	compass_object = p_factory->BuildGameObject("CompassObj.json");
	compass_objectives = compass_object->HasComponent<ObjectiveTracker>();

	if (compass_objectives != nullptr)
	{
		compass_objectives->SetPlayer(player->GetGameObject());

		GameObject* objective = p_factory->BuildGameObject("WaypointObj.json");
		Transform* objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(0.0f, track_height + 0.5f, -27.50f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(43.5f, track_height + 0.5f, -27.50f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(44.0f, track_height + 0.5f, -114.0f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(-50.0f, track_height + 0.5f, -114.0f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(-50.0f, track_height + 0.5f, -74.0f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(-10.0f, track_height + 0.5f, -74.0f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(-10.0f, track_height + 0.5f, 22.0f);
		compass_objectives->AddObjective(objective);

		objective = p_factory->BuildGameObject("WaypointObj.json");
		objective_tr = objective->HasComponent<Transform>();
		objective_tr->position = Vec3(0.0f, track_height + 0.5f, 22.0f);
		compass_objectives->AddObjective(objective);
	}
}

/*
* The prototype update function
*/
PROTOTYPE_INTERFACE PROTOTYPE_UPDATE(prototype_update) {
	//SIK_INFO("Updating Driving Prototype");

	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_R))
	{
		ResetGame();
	}

	player->Update(dt);
	countdown_timer->Update(dt);
}

/*
* The prototype destroy function
*/
PROTOTYPE_INTERFACE PROTOTYPE_END(prototype_end) {
	SIK_INFO("Ending Prototype");

	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();
	alloc.delete_object<CountdownTimer>(countdown_timer);
	alloc.delete_object<Player>(player);

	active_cam->SetControllerMode(ControlMode::VIEWER);
	active_cam->SetProjectionMode(ProjectionMode::PERSPECTIVE);
	p_scripting_manager->DeleteAllBehaviours();
	p_physics_manager->Clear();
	p_graphics_manager->Clear();
	p_graphics_manager->GetSkyToggle() = false;
	p_graphics_manager->GetBgToggle() = true;
	p_audio_manager->background_chanel->stop();

	p_graphics_manager->RemoveAllLocalLights();
	p_resource_manager->FreeDefaultAssets();
	p_game_obj_manager->DeleteAllGameObjects();
	p_gui_object_manager->DeleteAllGUIObjects();

	return;
}
