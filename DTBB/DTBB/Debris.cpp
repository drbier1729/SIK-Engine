#include "stdafx.h"

#include "Debris.h"

#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Engine/AudioManager.h"

#include "BaseState.h"
#include "ObjectHolder.h"
#include "Magnet.h"

#include <algorithm>

Debris::Debris():
	magnet{ nullptr },
	is_stuck{ false },
	collided_last_frame{ false },
	collided_this_frame{ false },
	timer{ 0.0f },
	orig_pos{ 0.0f },
	orig_ori{},
	gravity_scale{ 0.0f },
	transform_scale{ 0.0f },
	original_mass{ 0.0f },
	chase_speed{ 0.0f },
	max_chase_dist{ 0.0f }
{
}

Debris::~Debris() noexcept {
}

void Debris::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Debris>(json_value, this);
}

void Debris::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Debris>(json_value, this, alloc);
}

void Debris::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void Debris::OnCollide(GameObject* other) {
	if (magnet == nullptr) {
		return;
	}

	if (not is_stuck &&
		timer <= 0.0f) {
		// if other object is the target
		if (other == magnet->GetOwner() ||
			other == p_base_state->GetPlayerGameObjPtr()) {
			collided_this_frame = true;
		}
	}
}

void Debris::Link() {
	GameObject* p_player = p_base_state->GetPlayerGameObjPtr();
	ObjectHolder* p_obj_hol = p_player->HasComponent<ObjectHolder>();
	Vector<GameObject*> const& attached_gos = p_obj_hol->GetAttachedObjects();

	for (GameObject* obj : attached_gos) {
		if (obj->HasComponent<Magnet>()) {
			magnet = obj->HasComponent<Magnet>();
		}
	}

	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	if (!(p_rb && p_rb->motion_props)) { return; }
	gravity_scale = p_rb->motion_props->gravity_scale;
	original_mass = p_rb->motion_props->mass;

	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	transform_scale = p_tr->scale;

	orig_pos = p_rb ? p_rb->position : p_tr->position;
	orig_ori = p_rb ? p_rb->orientation : p_tr->orientation;
}

void Debris::Enable() {
	Link();
}

void Debris::Disable() {
}

void Debris::Reset() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	if (p_rb) {
		p_rb->position = orig_pos;
		p_rb->orientation = orig_ori;
	}
	else {
		p_tr->position = orig_pos;
		p_tr->orientation = orig_ori;
	}
}

void Debris::Update(Float32 dt) {
	if (magnet == nullptr) {
		return;
	}

	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	// collision enter
	if (collided_last_frame == false &&
		collided_this_frame == true) {
		// get stuck to magnet
		magnet->AddDebris(this);

		AudioUpdate();
	}

	collided_last_frame = collided_this_frame;
	collided_this_frame = false;
}

void Debris::FixedUpdate(Float32 dt) {
	if (magnet == nullptr) {
		return;
	}

	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	if (!(p_rb && p_rb->motion_props)) { return; }

	if (timer <= 0.0f) {
		if (not is_stuck) {
			// chase if magnet has slots
			if (magnet->HasFreeSlots()) {
				ChaseTarget(dt);
			}
			else {
				// scale up if smaller than original
				if (glm::all(glm::lessThan(p_tr->scale, transform_scale))) {
					p_tr->scale += Vec3{ dt };
				}
				// set original mass
				p_rb->motion_props->SetMass(original_mass);
			}
		}
		timer = 0.0f;
	}
	else {
		if (not is_stuck) {
			// scale up if smaller than original
			if (glm::all(glm::lessThan(p_tr->scale, transform_scale))) {
				p_tr->scale += Vec3{ dt };
			}
		}
	}
	p_rb->local_bounds.halfwidths = p_tr->scale;
	timer -= dt;
}

void Debris::ShotFromMagnet(Vec3 new_vel) {
	// magnet cannot pull this debris back for 3 seconds after shooting
	timer = 3.0f;

	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	if (!(p_rb && p_rb->motion_props)) { return; }

	// set new velocity
	p_rb->motion_props->linear_velocity = new_vel;
	
	p_rb->MakeTrigger(false);
	// set original gravity scale
	p_rb->motion_props->gravity_scale = gravity_scale;
	// set original mass
	p_rb->motion_props->SetMass(original_mass);
}

void Debris::SetStuck(Bool _is_stuck) {
	is_stuck = _is_stuck;
}

void Debris::AudioUpdate() {
	// Hit sound effect
	p_audio_manager->PlayAudio("TURRET_IMP"_sid,
		p_audio_manager->sfx_chanel_group,
		p_audio_manager->turret_impact_vol,
		1.5f,
		false,
		0);
}

Bool Debris::IsStuck() {
	return is_stuck;
}

void Debris::ChaseTarget(Float32 dt) {
	if (magnet == nullptr) {
		return;
	}

	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	RigidBody* magnet_rb = magnet->GetOwner()->HasComponent<RigidBody>();
	if (!(p_rb && p_rb->motion_props && magnet_rb)) { return; }

	Vec3 debris_to_target = magnet_rb->position - p_rb->position;
	Float32 length2 = glm::length2(debris_to_target);

	if (length2 < (max_chase_dist * max_chase_dist)) {
		// scaling value so speed increases as it gets closer to magnet
		Float32 inv_dist_scale = max_chase_dist * max_chase_dist / length2;
		inv_dist_scale = std::clamp(inv_dist_scale, 1.0f, 5.0f);

		Vec3 move_vec = glm::normalize(debris_to_target) * chase_speed * inv_dist_scale;
		p_rb->motion_props->linear_velocity = move_vec;

		// scale down if moving closer
		if (glm::all(glm::greaterThan(p_tr->scale, Vec3{ 0.5f }))) {
			p_tr->scale -= Vec3{ dt };
		}

		// change mass
		p_rb->motion_props->SetMass(1.0f);
	}
	else {
		// scale up if smaller than original
		if (glm::all(glm::lessThan(p_tr->scale, transform_scale))) {
			p_tr->scale += Vec3{ dt };
		}

		// set original mass
		p_rb->motion_props->SetMass(original_mass);
	}
}

BEGIN_ATTRIBUTES_FOR(Debris)
	DEFINE_MEMBER(Float32, chase_speed)
	DEFINE_MEMBER(Float32, max_chase_dist)
END_ATTRIBUTES