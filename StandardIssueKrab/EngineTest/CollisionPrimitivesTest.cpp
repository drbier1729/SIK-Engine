#include "stdafx.h"
#include "CollisionPrimitivesTest.h"

#include "Engine/RigidBody.h"
//#include "Engine/CollidablePrimitives.h"
//#include "Engine/CollisionProperties.h"
//#include "Engine/CollisionInfo.h"
//#include "Engine/MemoryResources.h"
#include "Engine/PhysicsManager.h"
#include "Engine/InputManager.h"
//#include "Engine/Mesh.h" // for drawing

static void PrintRB(RigidBody* rb) {
	SIK_WARN("---RigidBody {}---", (void*)rb);
	SIK_WARN("\tposition = {}, {}, {}", rb->position.x, rb->position.y, rb->position.z);
	SIK_WARN("\torientation = {} + {}i + {}j + {}k", rb->orientation.w, rb->orientation.x, rb->orientation.y, rb->orientation.z);
	
	
	if (rb->motion_props != nullptr) {
		Vec3 vel = rb->motion_props->linear_velocity;
		Vec3 ang = rb->motion_props->angular_velocity;
		SIK_WARN("\tlinear velocity = {}, {}, {} m/s", vel.x, vel.y, vel.z);
		SIK_WARN("\tangular velocity = {}, {}, {} rad/s", ang.x, ang.y, ang.z);
		SIK_WARN("\tlinear drag = {}", rb->motion_props->linear_damping);
		SIK_WARN("\tangular drag = {}", rb->motion_props->angular_damping);

		SIK_WARN("\tmass = {} kg", rb->motion_props->mass);
		SIK_WARN("\tgravity scale = {}", rb->motion_props->gravity_scale);
	}
	
	SIK_WARN("\tfriction = {}", rb->friction);
	SIK_WARN("\trestitution = {}", rb->restitution);
	
}

void CollisionPrimitivesTest::Setup(EngineExport* p_engine_exports) {
	p_physics_manager = p_engine_exports->p_engine_physics_manager;
	p_input_manager = p_engine_exports->p_engine_input_manager;

	using Collision::Collider;
	SIK_ERROR("This test currently does not work - it relied on hardcoded objects in PhysicsManager that have been removed.");
	SetError();

	//// RigidBody settings used to initialize the rbs
	//RigidBodyCreationSettings opts{};

	//opts.world_position = Vec3(5, 5, 0);
	//opts.collider_types[0] = Collider::Type::Sphere;
	//opts.collider_masses[0] = 50.0f;
	//opts.collider_position_offsets[0] = Vec3(0);
	//opts.collider_orientation_offsets[0] = Quat(1,0,0,0);
	//opts.motion_type = RigidBody::MotionType::Dynamic;
	//
	//p_physics_manager->CreateRigidBody(opts);
}

void CollisionPrimitivesTest::Run() {
	using namespace Collision;
	
	//auto& h = p_physics_manager->h;
	//auto& h2 = p_physics_manager->h2;
	//auto& s = p_physics_manager->s;
	//auto& s2 = p_physics_manager->s2;
	//auto& c = p_physics_manager->c;
	//auto& c2 = p_physics_manager->c2;

	//p_physics_manager->hull_cn = h.Collide(&h2);
	//p_physics_manager->sph_cn = s.Collide(&s2);
	//p_physics_manager->cap_cn = c.Collide(&c2);
	//p_physics_manager->sph_cap_cn = s.Collide(&c);


	//if (p_input_manager->IsKeyPressed(SDL_SCANCODE_BACKSPACE)) {
	//	SetPassed();
	//	SIK_INFO("Exiting physics test");
	//	return;
	//}

	//if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_SPACE)) {
	//	NextCollider();
	//}

	//// GATHER INPUT
	//Vec2 move{}, rot{};
	//// Rotations
	//if (p_input_manager->IsKeyPressed(SDL_SCANCODE_LSHIFT) ||
	//	p_input_manager->IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_W)) {
	//		rot.y += 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_S)) {
	//		rot.y -= 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
	//		rot.x -= 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
	//		rot.x += 1.0f;
	//	}
	//}
	//// Movement
	//else {
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_W)) {
	//		move.y += 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_S)) {
	//		move.y -= 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
	//		move.x -= 1.0f;
	//	}
	//	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
	//		move.x += 1.0f;
	//	}
	//	// Normalize
	//	if (move.x != 0 && move.y != 0) {
	//		move *= 0.707f;
	//	}
	//}

	//Collider* current = &p_physics_manager->s;
	//if (controlled == Collider::Type::Capsule) {
	//	current = &p_physics_manager->c;
	//}
	//else if (controlled == Collider::Type::Hull) {
	//	current = &p_physics_manager->h;
	//}
	//if (not current) { SetError(); return; }

	//Mat4 ori = current->GetRelativeRotationMat4();
	//Vec3 const fwd   = -ori[2];
	//Vec3 const right = ori[0];

	//Vec3 const pos = current->GetRelativePosition();
	//current->SetRelativePosition(pos + (move.y * fwd + move.x * right) * 0.1f);

	//ori = glm::rotate(ori, rot.x * 0.2f, Vec3(0, 1, 0));
	//ori = glm::rotate(ori, rot.y * 0.2f, right);
	//current->SetRelativeRotation(Mat3(ori));
	//current->UpdateWorldTransform();

	SetRunning();
}

void CollisionPrimitivesTest::Teardown() {
	SetPassed();
	return;
}