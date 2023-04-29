#include "stdafx.h"
#include "Engine/Factory.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GameObject.h"
#include "Engine/GraphicsManager.h"
#include "LocalLightsTest.h"

/*
* Sets up the Local Lights test.
* 1) Create the floor
* 2) Create 100 local lights
* Returns: void
*/
void LocalLightsTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

	GameObject* game_object;
	Transform* object_transform;

	//Build the floor to display lights
	game_object = p_factory->BuildGameObject("FloorObject.json");
	object_transform = game_object->HasComponent<Transform>();
	//Move it to slightly below origin
	object_transform->position = Vec3(0, -1, 0);
	////Rotate it to make it facing upwards
	//object_transform->orientation =
	//	glm::toQuat(glm::rotate(Mat4(1),
	//		glm::radians(-90.0f),
	//		Vec3(1.0f, 0.0f, 0.0f)));
	//Make it bigger
	object_transform->scale = Vec3(200.0f);
	
	//Add several local lights
	for (int i = -20; i <= 20; i += 2) {
		for (int j = -20; j <= 20; j += 2) {
			p_graphics_manager->AddLocalLight(Vec3(i, 1, j), Vec3(0.0f, 1.0f, 0.0f), 3.0f);
		}
	}

	for (int i = -8; i <= 8; i += 4) {
		for (int j = -8; j <= 8; j += 4) {
			p_graphics_manager->AddLocalLight(Vec3(i, 1, j), Vec3(1.0f, 1.0f, 1.0f), 5.0f);
		}
	}
	SetRunning();
}

/*
* Runs the Local Lights test
* Returns: void
*/
void LocalLightsTest::Run() {
}

/*
* Runs the teardown
* Deletes game object
* Deletes local lights
* Returns: void
*/
void LocalLightsTest::Teardown() {
	p_graphics_manager->RemoveAllLocalLights();
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
}
