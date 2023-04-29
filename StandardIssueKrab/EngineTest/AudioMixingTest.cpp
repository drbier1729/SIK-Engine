#include "stdafx.h"

#include "AudioMixingTest.h"

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
#include "Engine/AudioManager.h"


void AudioMixingTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_physics_manager = _p_engine_export_struct->p_engine_physics_manager;
	p_scripting_manager = _p_engine_export_struct->p_engine_scripting_manager;
	p_audio_manager = _p_engine_export_struct->p_engine_audio_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif

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

	// Build game objects

	Mesh* cone_temp = p_resource_manager->LoadMesh("sphere_primitive.fbx");

	Material* mat = p_resource_manager->LoadMaterial("lit.mat");

	// RigidBody used to initialize other rbs
	/*RigidBody init_rb{ .position = Vec3(0,0,0) };
	init_rb.SetMotionType(RigidBody::MotionType::Dynamic)
		.SetFlags(RigidBody::RB_FLAG_IS_VALID | RigidBody::RB_FLAG_IS_ACTIVE);*/


	GameObject* s2 = p_game_obj_manager->CreateGameObject("GameObject 2");

	s2->AddComponent(p_graphics_manager->CreateMeshRenderer(mat, cone_temp));

	// Add RigidBody
	/*init_rb.position = Vec3(5, 5, 0);
	s2->AddComponent(p_physics_manager->CreateRigidBody(init_rb));
	RigidBody* rb = s2->HasComponent<RigidBody>();
	SIK_ASSERT(rb != nullptr, "RigidBody 2 failed to add");
	MotionProperties* rb_mp = p_physics_manager->AddMotionProperties(rb);
	rb_mp->SetMass(10.0f);
	rb_mp->gravity_scale = 0;
	CollisionProperties* rb_cp = p_physics_manager->AddCollisionProperties(rb);
	CollidablePrimitive* rb_shape = p_physics_manager->AddCollidablePrimitive(rb, Sphere{});*/

	Transform* tr = s2->HasComponent<Transform>();
	SIK_ASSERT(tr != nullptr, "GO 2 must have Transform");

	controlled = s2;
	
	SetRunning();
}

void AudioMixingTest::Run() {

	RigidBody* controlled_rb = controlled->HasComponent<RigidBody>();
	
	if (controlled_rb->position.x > 50.0f) {
		//controlled_rb->motion_props->AddForce(Vec3(-50.0f, 0, 0) * 10.0f);
		p_input_manager->RumbleController(500, 5000, 1000);
	}
	else if (controlled_rb->position.x < -50) {
		//controlled_rb->motion_props->AddForce(Vec3(50, 0, 0) * 10.0f);
		p_input_manager->RumbleController(500, 5000, 1000);
	}
	else {
		Vec2 move{};
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
			move.x -= 1.0f;
			//controlled_rb->motion_props->AddForce(Vec3(move.x, 0, 0) * 200.0f);
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
			move.x += 1.0f;
			//controlled_rb->motion_props->AddForce(Vec3(move.x, 0, 0) * 200.0f);
		}
	}
	
	 
	Float32 volume = glm::clamp((controlled_rb->position.x + 50.0f) / 100.0f, 0.001f, 1.0f);
	p_audio_manager->SetVolume((1 - volume), p_audio_manager->background_chanel);

	p_audio_manager->SetVolume(volume, p_audio_manager->music_chanel);


	if (p_input_manager->AreControllersConnected())
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);

	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SIK_INFO("Ending Audio Mixing Test");
		SetPassed();
		return;
	}

	SetRunning();
	return;
}

void AudioMixingTest::Teardown() {
	p_game_obj_manager->DeleteAllGameObjects();
	SetPassed();
	return;
}
