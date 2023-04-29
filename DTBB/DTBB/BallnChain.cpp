#include "stdafx.h"
#include "BallnChain.h"
#include "Engine/Serializer.h"
#include "Engine/Factory.h"
#include "Engine/GameObjectManager.h"
#include "Engine/MotionProperties.h"
#include "Engine/ParticleSystem.h"
#include "Engine/RandomGenerator.h"
#include "Engine/PhysicsManager.h"

#include "CarController.h"
#include "Health.h"
#include "TurretEnemy.h"
#include "GenericCarEnemy.h"
#include "ObjectHolder.h"

void BallnChain::SetSegmentTransform(GameObject* prev_obj, GameObject* next_obj, GameObject* segment_object) {
	if (not prev_obj || not next_obj) { return; }

	if (not prev_obj->HasComponent<RigidBody>() ||
		not next_obj->HasComponent<RigidBody>())
	{
		return;
	}

	Vec3 position_prev = prev_obj->HasComponent<RigidBody>()->position;
	Vec3 position_next = next_obj->HasComponent<RigidBody>()->position;

	Vec3 diff_vector = position_next - position_prev;
	Vec3 diff_vector_halved = diff_vector * 0.5f;
	Vec3 diff_vector_norm = glm::normalize(diff_vector);
	Vec3 segment_len(0, 0, 1);

	Quat orientation = glm::quat(segment_len, diff_vector_norm);
	Vec3 position = position_prev + diff_vector_halved;
	Float32 scale = glm::length(diff_vector);

	segment_object->HasComponent<Transform>()->orientation = orientation;
	segment_object->HasComponent<Transform>()->position = position;
	segment_object->HasComponent<Transform>()->scale.z = scale;
}

BallnChain::BallnChain():
	chain_links_count{ 0 },
	ball_mass{ 0.0f },
	spring_coeff{ 0.0f },
	damper_coeff{ 0.0f },
	spring_rest_length{ 0.0f },
	p_parent_obj{ nullptr },
	anchor_obj{ nullptr },
	anchor_offset{ 0 },
	parent_obj_name{},
	p_emitter{ p_particle_system->NewEmitter() }
{
	// emitter settings
	p_emitter->gravity_scale = 0.0f;
	p_emitter->particles_per_sec = 50;
	p_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_emitter->color_gen_bnds = {
		.min = Vec4{ 0.82f, 0.81f, 0.78f, 1.0f },
		.max = Vec4{ 0.88f, 0.87f, 0.84f, 1.0f }
	};
	p_emitter->speed_gen_bnds = {
		.min = 0.1f,
		.max = 0.1f
	};
	p_emitter->theta = {
		.min = glm::radians(-90.0f),
		.max = glm::radians(90.0f)
	};
	p_emitter->phi = {
		.min = glm::radians(-10.0f),
		.max = glm::radians(-170.0f)
	};
	p_emitter->lifetime_secs_gen_bnds = {
		.min = 0.1f,
		.max = 0.3f
	};
	p_emitter->uniform_scale_gen_bnds = {
		.min = 0.2f,
		.max = 0.4f
	};
}

BallnChain::~BallnChain() noexcept {
	p_particle_system->EraseEmitter(p_emitter);

	for (auto& obj : chain_link_objects) {
		p_game_obj_manager->DeleteGameObject(obj);
	}
	chain_link_objects.clear();

	for (auto& obj : chain_segment_objects) {
		p_game_obj_manager->DeleteGameObject(obj);
	}
	chain_segment_objects.clear();

	p_game_obj_manager->DeleteGameObject(anchor_obj);
}

void BallnChain::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<BallnChain>(json_value, this);
	parent_obj_name = json_value.FindMember("parent_obj_name")->value.GetString();

	if (chain_links_count < 3) {
		SIK_WARN("Minimum number of chain links is 3. Setting the chain link count to 3");
		chain_links_count = 3;
	}
	
	//Extra chain segment since there needs to be one segment between the final chain link
	// and the anchored object
	chain_segment_objects.push_back(
		p_factory->BuildGameObject("ChainSegment.json"));

	//Create an object for each chain link and maintain a list of them.
	for (Uint8 i = 0; i < chain_links_count; ++i) {
		chain_link_objects.push_back(
			p_factory->BuildGameObject("ChainLink.json"));

		chain_segment_objects.push_back(
			p_factory->BuildGameObject("ChainSegment.json"));
	}
	//Create the anchor object which will always be at the 
	//specified offset position to the parent
	anchor_obj = p_factory->BuildGameObject("ChainAnchor.json");
}

void BallnChain::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<BallnChain>(json_value, this, alloc);
}

void BallnChain::Modify(rapidjson::Value const& json_value) {
	DeserializeReflectable<BallnChain>(json_value, this);

	auto itr = json_value.FindMember("parent_obj_name");
	if (itr != json_value.MemberEnd()) {
		parent_obj_name = itr->value.GetString();
	}

	if (chain_link_objects.size() < chain_links_count) {
		AddChainLink(chain_links_count - chain_link_objects.size());
	}
	else if (chain_link_objects.size() > chain_links_count) {
		RemoveChainLink(chain_link_objects.size() - chain_links_count);
	}
}

void BallnChain::Link() {
	auto& go_pool = p_game_obj_manager->GetGameObjectContainer();

	for (auto r = go_pool.all(); not r.is_empty(); r.pop_front()) {
		GameObject& go_ref = r.front();
		if (go_ref.GetName().compare(parent_obj_name) == 0) {
			p_parent_obj = &go_ref;
			break;
		}
	}
	
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!(rb && rb->motion_props)) { return; }
	rb->motion_props->SetMass(ball_mass);
	ResetBallPosition();
}

void BallnChain::Enable() {
	for (auto& obj : chain_link_objects) {
		obj->Enable();
	}

	for (auto& obj : chain_segment_objects) {
		obj->Enable();
	}

	anchor_obj->Enable();
	ResetBallPosition();
}

void BallnChain::Disable() {
	for (auto& obj : chain_link_objects) {
		obj->Disable();
	}

	for (auto& obj : chain_segment_objects) {
		obj->Disable();
	}

	anchor_obj->Disable();

	p_emitter->is_active = false;
}

void BallnChain::AddChainLink(Uint8 add_count) {

	static constexpr Uint8 max_links = 12;

	if (chain_links_count == max_links) { return; }

	if (add_count + chain_links_count > max_links) {
		add_count = max_links - chain_links_count;
	}

	if (not p_parent_obj) { return; }

	Transform* parent_transform = p_parent_obj->HasComponent<Transform>();
	
	Float32 x_offset = chain_link_objects[0]->HasComponent<RigidBody>()->bounds.halfwidths.x * 2.0f;

	for (Uint8 i = 0; i < add_count; ++i) {
		{
			chain_link_objects.push_back(
				p_factory->BuildGameObject("ChainLink.json"));
			RigidBody* rb = chain_link_objects.back()->HasComponent<RigidBody>();
			if (!rb) { continue; }
			rb->position.x = parent_transform->position.x;
			rb->position.x += x_offset * (i + 1);
		}
		{
			chain_segment_objects.push_back(
				p_factory->BuildGameObject("ChainSegment.json"));
			RigidBody* rb = chain_segment_objects.back()->HasComponent<RigidBody>();
			if (!rb) { continue; }
			rb->position.x = parent_transform->position.x;
			rb->position.x += x_offset * (i + 1);
		}
	}

	chain_links_count = static_cast<Uint8>(chain_link_objects.size());
}

void BallnChain::RemoveChainLink(Uint8 remove_count) {
	
	if (remove_count == 0) { 
		SIK_WARN("Useless call. Remove count was 0");
		return; 
	}

	// Use a switch statement b/c dealing with unsigned arithmetic is a pain
	switch (chain_links_count)
	{
		break; case 0:
			   case 1:
			   case 2: 
		{
			SIK_ASSERT(chain_links_count >= 3, "Must always have at least 3 chain links");
			return;
		}
		break; case 3: { 
			return; 
		}
		break; case 4: {
			remove_count = 1;
		}
		break; case 5: {
			if (remove_count > 2) {
				remove_count = 2;
			}
		}
	}

	if (remove_count > chain_links_count) {
		remove_count = chain_links_count - 3;
	}

	GameObject* obj_to_remove;
	for (Uint8 i = 0; i < remove_count; ++i) {
		obj_to_remove = chain_link_objects.back();
		chain_link_objects.pop_back();
		p_game_obj_manager->DeleteGameObject(obj_to_remove);

		obj_to_remove = chain_segment_objects.back();
		chain_segment_objects.pop_back();
		p_game_obj_manager->DeleteGameObject(obj_to_remove);
	}

	chain_links_count = static_cast<Uint8>(chain_link_objects.size());
}

void BallnChain::UpdateChainLength()
{
	if (not p_parent_obj) { return; }

	CarController* controller = p_parent_obj->HasComponent<CarController>();
	if (not controller) { return; }

	Bool const add_link = controller->GrowChain();
	Bool const rm_link = controller->ShrinkChain();

	if (add_link && rm_link) { return; }

	if (add_link) {
		AddChainLink();
	}

	else if (rm_link) {
		RemoveChainLink();
	}
}

// DOESN'T WORK
void BallnChain::ApplyBounceForce(RigidBody* ball_rb)
{
	//static constexpr Float32 lo = 0.1f;
	//static constexpr Float32 hi = 0.3f;
	//static UniformRandFloat32 rng{};

	//Vec3 const rand_bounce_impulse = Vec3(0, 1, 0) * rng.GenInRange(lo, hi);
	//Float32 const speed2 = glm::length2( ball_rb->motion_props->linear_velocity );

	//// If the ball is colliding with the ground
	//if (ball_rb->position.y <= ball_rb->bounds.halfwidths.y + 0.2f && speed2 > 25.0f) {
	//	ball_rb->motion_props->linear_velocity += rand_bounce_impulse;
	//}
}

void BallnChain::FixedUpdate(Float32 dt) {
	//Set the anchor point at the offset and apply the parent rotation
	RigidBody* anchor_rb = anchor_obj->HasComponent<RigidBody>();
	RigidBody* parent_rb = p_parent_obj->HasComponent<RigidBody>();

	Vec3 anchor_pos = parent_rb->LocalToWorld(anchor_offset);
	anchor_rb->position = anchor_pos;
	
	GameObject* ball_obj = GetOwner();
	if (not ball_obj) { return; }

	RigidBody* ball_rb = ball_obj->HasComponent<RigidBody>();
	if (not ball_rb) { return; }

	//Check if the Ball is really far away from the car
	Float32 unstable_ball_length = (spring_rest_length * chain_links_count * 10.0f);
	if (glm::length(ball_rb->position - anchor_rb->position) > unstable_ball_length) {
		ResetBallPosition();
	}

	//For the first link, use the parent object for position and velocity
	ApplySpringForce(
		anchor_obj,
		chain_link_objects[0],
		chain_link_objects[1]
	);

	//For the middle links we can use the surrounding links
	for (Uint8 i = 1; i < chain_links_count - 1; ++i) {
		ApplySpringForce(
			chain_link_objects[i - 1],
			chain_link_objects[i],
			chain_link_objects[i + 1]
		);
	}

	//For the last link, use the ball object for the position and velocity
	ApplySpringForce(
		chain_link_objects[chain_links_count - 2],
		chain_link_objects[chain_links_count - 1],
		GetOwner()
	);

	//Finally for the ball, we can consider the force only being applyied from the last link
	ApplySpringForce(
		chain_link_objects[chain_links_count - 1],
		GetOwner(),
		GetOwner()
	);

	// Correct wrecking ball position
	MoveWreckingBallAwayFromParent(ball_rb, parent_rb);

	// Ball targets closest enemy
	SeekClosestEnemy(ball_rb);

	// Add/remove chain links based on user input
	//UpdateChainLength();

	// Emulate bouncing and skidding -- DOES NOT WORK
	//ApplyBounceForce(ball_rb);

	// emitter
	EmitterChecks();

	SetSegmentTransforms();

	//Correct the scale according to the spring rest length
	for (auto& obj : chain_segment_objects) {
		obj->HasComponent<Transform>()->scale.z *= spring_rest_length/1.5;
	}
}

void BallnChain::Update(Float32 dt) {
#ifdef _ENABLE_EDITOR
	if (static_cast<Uint8>(chain_link_objects.size()) < chain_links_count) {
		AddChainLink(chain_links_count - static_cast<Uint8>(chain_link_objects.size()));
	}
	else if (static_cast<Uint8>(chain_link_objects.size()) > chain_links_count) {
		RemoveChainLink(static_cast<Uint8>(chain_link_objects.size()) - chain_links_count);
	}
#endif
}

Int32 BallnChain::CalculateDamage() {

	//To-Do : Fine tuning the damage calculation

	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!(rb && rb->motion_props)) { return 0; }

	Float32 vel = glm::length(rb->motion_props->linear_velocity);
	
	Float32 dmg_scaling = 0.002f;

	return static_cast<Int32>(vel * ball_mass * dmg_scaling);
}

void BallnChain::OnCollide(GameObject* other) {
	if (other == p_parent_obj)
		return;

	CarController* cc = other->HasComponent<CarController>();

	if (cc != nullptr) {
		Health* h = other->HasComponent<Health>();
		Int32 dmg_dealt = CalculateDamage();
		SIK_INFO("Dealt damage \"{}\"", dmg_dealt);
		h->TakeDamage(dmg_dealt);
	}
}

void BallnChain::Reset() {
	ResetBallPosition();
}

Vec3 const& BallnChain::GetAnchorOffset() {
	return anchor_offset;
}

void BallnChain::SetAnchorOffset(Vec3 const& _anchor_offset) {
	anchor_offset = _anchor_offset;
}

/*
* Calculates and applies spring forces on the "curr_obj"
* based on the position and velocities of the "prev_obj" and "next_obj"
* Returns: void
*/
void BallnChain::ApplySpringForce(GameObject* prev_obj, GameObject* curr_obj, GameObject* next_obj) {
	Vec3 qi_prev, qi_next, qi_curr; //Positions
	Vec3 vi_prev, vi_next, vi_curr; //Velocities

	if (not prev_obj || not curr_obj || not next_obj) { return; }

	RigidBody* prev_rb = prev_obj->HasComponent<RigidBody>();
	RigidBody* curr_rb = curr_obj->HasComponent<RigidBody>();
	RigidBody* next_rb = next_obj->HasComponent<RigidBody>();

	if (not (prev_rb && prev_rb->motion_props) ||
		not (curr_rb && curr_rb->motion_props) ||
		not (next_rb && next_rb->motion_props))
	{
		return;
	}

	qi_prev = prev_rb->position;
	vi_prev = prev_rb->motion_props->linear_velocity;

	qi_curr = curr_rb->position;
	vi_curr = curr_rb->motion_props->linear_velocity;

	qi_next = next_rb->position;
	vi_next = next_rb->motion_props->linear_velocity;


	Vec3 diff = qi_prev - qi_curr;
	Vec3 n_diff = Vec3(0);
	Float32 diff_mag = glm::length(diff);
	if (diff_mag != 0.0f) {
		n_diff = glm::normalize(diff) * spring_rest_length;
	}

	Vec3 prev_force = (n_diff * (diff_mag - spring_rest_length)) * spring_coeff;


	diff = qi_next - qi_curr;
	n_diff = Vec3(0);
	diff_mag = glm::length(diff);
	if (diff_mag != 0.0f) {
		n_diff = glm::normalize(diff) * spring_rest_length;
	}
	Vec3 next_force = (n_diff * (diff_mag - spring_rest_length)) * spring_coeff;
	
	//Reset vertical force to make sure it always stays on the XZ plane
	prev_force.y = 0;
	next_force.y = 0;

	curr_rb->AddForce(prev_force + next_force);
}

void BallnChain::EmitterChecks() {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!(rb && rb->motion_props)) { return; }

	Float32 vel = glm::length(rb->motion_props->linear_velocity);
	if (vel > 40.0f) {
		p_emitter->is_active = true;

		// position
		p_emitter->position = rb->position;
		// orientation
		p_emitter->orientation = rb->orientation;
	}
	else {
		p_emitter->is_active = false;
	}
}

void BallnChain::MoveWreckingBallAwayFromParent(RigidBody* owner_rb, RigidBody* parent_rb)
{
	static constexpr Float32 repel_radius = 3.0f;
	static constexpr Float32 repel_radius_sqr = repel_radius * repel_radius;
	
	if (not owner_rb || not parent_rb) { return; }

	// compute sqr distance from wrecking ball AABB to center of parent
	Float32 const d2 = owner_rb->bounds.DistSquaredFromPoint(parent_rb->position);
	
	// wrecking ball AABB is withing repel_radius of parent
	if (d2 < repel_radius_sqr) {

		// apply position correction to move the ball away from parent
		Vec3 const repel_dir = glm::normalize( owner_rb->position - parent_rb->position );
		owner_rb->position += repel_dir * (repel_radius - glm::sqrt(d2));
	}

}


void BallnChain::SeekClosestEnemy(RigidBody* ball_rb) {
	if (not ball_rb) { return; }

	p_physics_manager->ForEachInRadius(8.0f, ball_rb->position, 
		[ball_rb](RigidBody const& rb) -> Bool {
			if (not rb.owner || ball_rb->owner == rb.owner) { return true; }

			Bool found_enemy = false;
			if (GenericCarEnemy* c = rb.owner->HasComponent<GenericCarEnemy>(); c) {
				found_enemy = true;
			}
			else if (ObjectHolder* h = rb.owner->HasComponent<ObjectHolder>(); h) {
					auto const& attached = h->GetAttachedObjects();
					for (auto&& go : attached) {
						if (not go) { continue; }

						if (TurretEnemy* t = go->HasComponent<TurretEnemy>(); t) {
							if (t->IsAlive() && not t->IsDying()) {
								found_enemy = true;
							}
						}
					}
			}

			if (found_enemy) {
				if (MotionProperties* mp = ball_rb->motion_props; mp) {
					Vec3 disp = rb.position - ball_rb->position;
					Float32 const invD2 = 1.0f / glm::length2(disp);
					disp *= glm::sqrt(invD2);

					Float32 const speed2 = glm::length2(mp->linear_velocity);

					ball_rb->AddForce(disp * invD2 * speed2 * 100.0f);
				}
			}

			return !found_enemy;
		});


}

void BallnChain::SetSegmentTransforms() {
	SetSegmentTransform(
		anchor_obj,
		chain_link_objects[0],
		chain_segment_objects[0]);

	for (Uint8 i = 0; i < chain_links_count - 1; ++i) {
		SetSegmentTransform(
			chain_link_objects[i],
			chain_link_objects[i + 1],
			chain_segment_objects[i + 1]);
	}

	SetSegmentTransform(
		chain_link_objects[chain_links_count - 1],
		GetOwner(),
		chain_segment_objects[chain_links_count]);

}

void BallnChain::ResetBallPosition() {
	Transform* anchor_transform = anchor_obj->HasComponent<Transform>();
	RigidBody* anchor_rigidbody = anchor_obj->HasComponent<RigidBody>();
	Transform* parent_transform = p_parent_obj->HasComponent<Transform>();
	RigidBody* parent_rigidbody = p_parent_obj->HasComponent<RigidBody>();

	Vec3 anchor_pos = parent_rigidbody->LocalToWorld(anchor_offset);
	anchor_rigidbody->position = anchor_pos;

	Float32 z_offset = spring_rest_length;

	for (Uint8 i = 0; i < chain_links_count; ++i) {
		//Offset the chain link
		GameObject* go = chain_link_objects[i];
		RigidBody* rb = go->HasComponent<RigidBody>();
		if (!(go && rb && rb->motion_props)) { continue; }
		//Start from the anchor offset point
		Vec3 link_local_offset = anchor_offset;
		//Positive Z is behind the car. So move it back by the number of chain links.
		link_local_offset.z += z_offset * (i + 1);
		Vec3 link_world_offset = parent_rigidbody->LocalToWorld(link_local_offset);
		rb->position = link_world_offset;
		rb->motion_props->linear_velocity = Vec3(0.0f);

		//Offset the chain segments in the same way
		go = chain_segment_objects[i];
		Transform* tr = go->HasComponent<Transform>();
		tr->position = link_world_offset;
		tr->scale.z = spring_rest_length;
	}

	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!(rb && rb->motion_props)) { return; }
	Vec3 ball_local_offset = anchor_offset;
	ball_local_offset.z += z_offset * (chain_links_count + 1);
	rb->position = parent_rigidbody->LocalToWorld(ball_local_offset);
	rb->motion_props->linear_velocity = Vec3(0.0f);

}

BEGIN_ATTRIBUTES_FOR(BallnChain)
DEFINE_MEMBER(Uint8, chain_links_count)
DEFINE_MEMBER(Float32, ball_mass)
DEFINE_MEMBER(Float32, spring_coeff)
DEFINE_MEMBER(Float32, damper_coeff)
DEFINE_MEMBER(Float32, spring_rest_length)
DEFINE_MEMBER(Vec3, anchor_offset)
END_ATTRIBUTES