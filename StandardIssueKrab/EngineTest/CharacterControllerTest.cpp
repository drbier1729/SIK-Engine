#include "stdafx.h"
#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GameObject.h"
#include "Engine/InputAction.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/RenderCam.h"
#include "Engine/Transform.h"

#include "CharacterControllerTest.h"


/*
* Sets up the Character controller test.
* 1) Create a game object that can be controlled with an action map
* 2) Disable camera control
* Returns: void
*/
void CharacterControllerTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_scripting_manager = _p_engine_export_struct->p_engine_scripting_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

	GameObject* game_object;
	Transform* object_transform;

	//Build the floor to display shadows
	game_object = p_factory->BuildGameObject("FloorObject.json");
	object_transform = game_object->HasComponent<Transform>();
	//Move it to slightly below origin
	object_transform->position = Vec3(0, -1, 0);
	//Rotate it to make it facing upwards
	object_transform->orientation =
		glm::toQuat(glm::rotate(Mat4(1),
			glm::radians(-90.0f),
			Vec3(1.0f, 0.0f, 0.0f)));
	//Make it bigger
	object_transform->scale = Vec3(200.0f);

	//Create the controllable game object
	SIK_INFO("Create the controllable game object");
	p_obj = p_factory->BuildGameObject("CharacterControllerObj.json");

	SIK_INFO("Disable camera control");
	RenderCam* active_cam = p_graphics_manager->GetPActiveCam();
	active_cam->SetControllerMode(ControlMode::NONE);

	action_map = new InputAction("character_controls.json");

	SetRunning();
}

/*
* Runs the CharacterControllerTest test
* Steps:
* 1) Update the game object
* 2) Check for inputs
* Returns: void
*/
void CharacterControllerTest::Run(){
	if (p_obj == nullptr)
		return;

	Transform* object_transform = p_obj->HasComponent<Transform>();

	if (action_map->IsActionPressed(InputAction::Actions::UP)) {
		float new_z = object_transform->position.z;
		new_z -= 0.5;
		object_transform->position.z = new_z;
	}
	if (action_map->IsActionPressed(InputAction::Actions::DOWN)) {
		float new_z = object_transform->position.z;
		new_z += 0.5;
		object_transform->position.z = new_z;
	}
	if (action_map->IsActionPressed(InputAction::Actions::LEFT)) {
		float new_x = object_transform->position.x;
		new_x -= 0.5;
		object_transform->position.x = new_x;
	}
	if (action_map->IsActionPressed(InputAction::Actions::RIGHT)) {
		float new_x = object_transform->position.x;
		new_x += 0.5;
		object_transform->position.x = new_x;
	}
	if (action_map->IsActionPressed(InputAction::Actions::UP_ALT)) {
		float new_y = object_transform->position.y;
		new_y += 0.5;
		object_transform->position.y = new_y;
	}
	if (action_map->IsActionPressed(InputAction::Actions::DOWN_ALT)) {
		float new_y = object_transform->position.y;
		new_y -= 0.5;
		object_transform->position.y = new_y;
	}
	if (action_map->IsActionPressed(InputAction::Actions::ACTION_1)) {
		p_game_obj_manager->DeleteGameObject(p_obj);
		p_obj = nullptr;
	}
}

/*
* Runs the teardown
* Deletes game object
* Returns: void
*/
void CharacterControllerTest::Teardown() {
	p_graphics_manager->RemoveAllLocalLights();
	p_game_obj_manager->DeleteAllGameObjects();
	SIK_INFO("Enable camera control");
	RenderCam* active_cam = p_graphics_manager->GetPActiveCam();
	active_cam->SetControllerMode(ControlMode::VIEWER);
	SetPassed();
	delete action_map;
}
