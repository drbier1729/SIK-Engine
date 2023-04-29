#include "stdafx.h"

#include "ReallyStressfulTest.h"

#include "Engine/Factory.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GameObject.h"
#include "Engine/InputManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/PhysicsManager.h"
#include "Engine/Transform.h"
#include "Engine/RigidBody.h"

#include <random>

void ReallyStressfulTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_physics_manager = _p_engine_export_struct->p_engine_physics_manager;
	p_scripting_manager = _p_engine_export_struct->p_engine_scripting_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

	Vector<GameObject*> all_objects;
	GameObject* game_object;
	Transform* object_transform;

	float x = -50.0f;
	float y = 50.0f;
	int counter = 0;

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
	object_transform->scale = Vec3(20.0f);

	// Add 101 game objects
	String game_object_name;
	for (Uint16 i = 0; i < 101; ++i) {
		Int8 obj_n = std::rand() / ((RAND_MAX + 1u) / 4);
		game_object_name = "GameObjectTest_" + std::to_string(obj_n) + ".json";
		game_object = p_factory->BuildGameObject(game_object_name.c_str());
		all_objects.push_back(game_object);

		object_transform = game_object->HasComponent<Transform>();

		object_transform->position = {x, y, 0.0f};
		counter++;

		if (counter < 10) {
			x += 10.0f;
		}
		else {
			x = -50.0f;
			y -= 10.0f;
			counter = 0;
		}
	}

	// Erase 55 random ones
	for (Uint16 i = 0; i < 55; ++i) {
		
		Uint16 const n = std::rand() % all_objects.size();
		
		p_game_obj_manager->DeleteGameObject(all_objects[n]);

		all_objects[n] = all_objects.back();
		all_objects.pop_back();
	}

	// Add 101 more
	for (Uint16 i = 0; i < 101; ++i) {
		Int8 obj_n = std::rand() / ((RAND_MAX + 1u) / 4);
		game_object_name = "GameObjectTest_" + std::to_string(obj_n) + ".json";
		game_object = p_factory->BuildGameObject(game_object_name.c_str());
		all_objects.push_back(game_object);

		object_transform = game_object->HasComponent<Transform>();

		object_transform->position = { x, y, 0.0f };
		counter++;

		if (counter < 10) {
			x += 10.0f;
		}
		else {
			x = -50.0f;
			y -= 10.0f;
			counter = 0;
		}
	}

	// Erase 100 random ones
	for (Uint16 i = 0; i < 100; ++i) {

		Uint16 const n = std::rand() % all_objects.size();

		p_game_obj_manager->DeleteGameObject(all_objects[n]);

		all_objects[n] = all_objects.back();
		all_objects.pop_back();
	}

	// Add 40 more
	for (Uint16 i = 0; i < 40; ++i) {
		Int8 obj_n = std::rand() / ((RAND_MAX + 1u) / 4);
		game_object_name = "GameObjectTest_" + std::to_string(obj_n) + ".json";
		game_object = p_factory->BuildGameObject(game_object_name.c_str());
		all_objects.push_back(game_object);

		object_transform = game_object->HasComponent<Transform>();

		object_transform->position = { x, y, 0.0f };
		counter++;

		if (counter < 10) {
			x += 10.0f;
		}
		else {
			x = -50.0f;
			y -= 10.0f;
			counter = 0;
		}
	}

	SetRunning();
}

void ReallyStressfulTest::Run() {

	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SIK_INFO("Ending Really Stressful Test");
		SetPassed();
		return;
	}

	SetRunning();
	return;
}

void ReallyStressfulTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
