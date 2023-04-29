#include "stdafx.h"

#include "Engine/MemoryResources.h"
#include "Engine/GameObjectManager.h"
#include "Engine/ResourceManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/PhysicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/Factory.h"
#include "Engine/TestComp.h"

#include "GameObjectTest.h"

/*
* Sets up the GameObject test.
* Initializes the game_obj_manager pointer
* Returns: TestState
*/
void GameObjectTest::Setup(EngineExport* _p_engine_export_struct) {
	p_game_obj_manager = _p_engine_export_struct->p_engine_game_obj_manager;
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
	p_physics_manager  = _p_engine_export_struct->p_engine_physics_manager;
	p_input_manager  = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	p_factory = _p_engine_export_struct->p_engine_factory;

#ifdef _DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif // DEBUG


	// Build game objects

	Mesh* cone_temp = p_resource_manager->LoadMesh("sphere_primitive.fbx");

	Material* mat = p_resource_manager->LoadMaterial("lit.mat");

	//// RigidBody used to initialize other rbs
	RigidBodyCreationSettings init_rb{ 
		.position = Vec3(-5,5,0),

		.collider_parameters = { 
			{
				.type = Collision::Collider::Type::Capsule,
				.mass = 10.0f,
				.capsule_args = {
					.length = 2.0f,
					.radius = 1.0f
				}
			}
		},

		.motion_type = RigidBody::MotionType::Dynamic,
		.gravity_scale = 0.5f,
		.restitution = 1.0f
	};

	SIK_INFO("1. Creating first game object");
	GameObject* s1 = p_game_obj_manager->CreateGameObject("GameObject 1");
	
	// Add MeshRenderer
	s1->AddComponent(p_graphics_manager->CreateMeshRenderer(mat, cone_temp));
	
	// Add RigidBody
	s1->AddComponent(p_physics_manager->CreateRigidBody(init_rb));
	RigidBody* rb = s1->HasComponent<RigidBody>();
	SIK_ASSERT(rb != nullptr, "RigidBody 1 failed to add");

	Transform* tr = s1->HasComponent<Transform>();
	SIK_ASSERT(tr != nullptr, "GO 1 must have Transform");


	SIK_INFO("2. Creating the second game object");
	GameObject* s2 = p_game_obj_manager->CreateGameObject("GameObject 2");
	
	// Add MeshRenderer
	s2->AddComponent(p_graphics_manager->CreateMeshRenderer(mat, cone_temp));
	
	//// Add RigidBody
	init_rb.position = Vec3(5, 5, 0);
	init_rb.gravity_scale = 0.0f;
	init_rb.collider_parameters[0] = {
		.type = Collision::Collider::Type::Sphere,
		.mass = 10.0f,
		.sphere_args = {
			.radius = 1.0f 
		}
	};
	s2->AddComponent(p_physics_manager->CreateRigidBody(init_rb));
	rb = s2->HasComponent<RigidBody>();
	SIK_ASSERT(rb != nullptr, "RigidBody 2 failed to add");
	

	tr = s2->HasComponent<Transform>();
	SIK_ASSERT(tr != nullptr, "GO 2 must have Transform");

	s2->AddComponent(new TestComp2());

	controlled = s2;



	SIK_INFO("3. Creating the third game object");
	GameObject* s3 = p_game_obj_manager->CreateGameObject("GameObject 3");
	
	tr = s3->HasComponent<Transform>();
	SIK_ASSERT(tr != nullptr, "GO 3 must have Transform");
	tr->scale = Vec3(30, 1, 30);
	tr->position = Vec3(0, -5, 0);
	
	s3->AddComponent(p_graphics_manager->CreateMeshRenderer(mat, cone_temp));
	
	init_rb.position = tr->position;
	init_rb.motion_type = RigidBody::MotionType::Static;
	init_rb.collider_parameters[0] = {
				.type = Collision::Collider::Type::Hull,
				.mass = 0.0f,
				.hull_args = {
					.halfwidths = tr->scale,
					.is_box = true
				}
			};
	s3->AddComponent(p_physics_manager->CreateRigidBody(init_rb));
	rb = s3->HasComponent<RigidBody>();
	SIK_ASSERT(rb != nullptr, "RigidBody 3 failed to add");
	
	GameObject* s5 = p_factory->BuildGameObject("PhongObject.json");
	GameObject* s6 = p_factory->BuildGameObject("BlinnPhongObject.json");
	GameObject* s7 = p_factory->BuildGameObject("BRDFObject.json");

	
	
	SetRunning();
}

void GameObjectTest::Run() {
	
	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SetPassed();
		return;
	}


	// Rotations
	RigidBody* controlled_rb = controlled->HasComponent<RigidBody>();
	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_LSHIFT) ||
		p_input_manager->IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
		Vec2 rot{};
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_W)) {
			rot.y += 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_S)) {
			rot.y -= 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
			rot.x -= 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
			rot.x += 1.0f;
		}
		controlled_rb->orientation = glm::rotate(controlled_rb->orientation, 0.02f * rot.x, Vec3(0, 0, 1));
		controlled_rb->orientation = glm::rotate(controlled_rb->orientation, 0.02f * rot.y, Vec3(1, 0, 0));
	}

	// Force-based movement
	else {
		Vec2 move{};
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_W)) {
			move.y += 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_S)) {
			move.y -= 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
			move.x -= 1.0f;
		}
		if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
			move.x += 1.0f;
		}
		controlled_rb->AddForce(Vec3(move.x, move.y, 0) * 200.0f);
	}

	SetRunning();
	return;
}

/*
* Runs the teardown
* Deletes all game objects
* Returns: TestState
*/
void GameObjectTest::Teardown() {
	SIK_INFO("6. Deleting all game objects");
	p_game_obj_manager->DeleteAllGameObjects();
	
	SetPassed();
	return;
}
