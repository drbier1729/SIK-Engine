#include "stdafx.h"

#include "Collectable.h"

#include "Inventory.h"

#include "PlayerCharacter.h"
#include "BaseState.h"
#include "GamePlayState.h"

#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Engine/EasingFunction.h"
#include "Engine/Serializer.h"
#include "Engine/ParticleSystem.h"
#include "Engine/AudioManager.h"


Collectable::Collectable():
	p_emitter{ p_particle_system->NewEmitter() },
	is_collected{ false },
	c_type{ CTypes::Resource1 },
	elapsed_time{ 0.0f },
	rising_speed{ 0.0f },
	chase_speed{ 0.0f },
	apex_threshold{ 0.0f },
	starting_height{ 0.0f }
{
	// emitter settings
	p_emitter->gravity_scale = 0.0f;
	p_emitter->particles_per_sec = 20;
	p_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_emitter->color_gen_bnds = {
		.min = Vec4{ 0.95f, 0.74f, 0.12f, 1.0f },
		.max = Vec4{ 0.97f, 0.74f, 0.12f, 1.0f }
	};
	p_emitter->speed_gen_bnds = {
		.min = 0.1f,
		.max = 0.2f
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
		.min = 0.1f,
		.max = 0.2f
	};
	p_emitter->is_active = true;
}

Collectable::~Collectable() noexcept {
	p_particle_system->EraseEmitter(p_emitter);
}

std::unordered_map<StringID, Collectable::CTypes> Collectable::string_types_map{
	{"Resource1"_sid, Collectable::CTypes::Resource1},
	{"Resource2"_sid, Collectable::CTypes::Resource2}
};

void Collectable::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Collectable>(json_value, this);

	auto res_iter = string_types_map.find(
		ToStringID(json_value.FindMember("Type")->value.GetString()));

	SIK_ASSERT(res_iter != string_types_map.end(), "Failed to find collectable type");

	c_type = res_iter->second;
}

void Collectable::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Collectable>(json_value, this, alloc);
}

void Collectable::Enable() {
	p_emitter->is_active = true;
	is_collected = false;
}

void Collectable::Disable() {
	p_emitter->is_active = false;
}

void Collectable::OnCollide(GameObject* other) {

	// disable collision before it float to the top 
	if (elapsed_time < apex_threshold) return;

	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (not rb->IsEnabled())
		return;

	if (other != p_base_state->GetPlayerGameObjPtr())
		return;

	is_collected = true;
	p_emitter->is_active = false;

	GameObject* owner = GetOwner();
	if (owner) {
		owner->Disable();
	}

	// Health Collectible
	if (c_type == CTypes::Resource2) {
		PlayerCharacter* player_char = other->HasComponent<PlayerCharacter>();
		// reduce negative = add
		player_char->TakeDamage(-10);
	}
	else { // Resource Collectible
		p_base_state->GetGamePlayState()->RemoveObject(owner);
		Inventory* obj_inv = other->HasComponent<Inventory>();
		if (obj_inv) {
			obj_inv->AddCollectable(this);
		}

		p_audio_manager->PlayAudio("COLLECTIBLE"_sid,
			p_audio_manager->sfx_chanel_group,
			20.0f,
			1.0f,
			false,
			0);
	}

	// Update the game play state
	auto* gameplay_state = p_base_state->GetGamePlayState();
	if (gameplay_state) {
		gameplay_state->RemoveObject(GetOwner());
	}
}

void Collectable::Update(Float32 dt) {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!(rb && rb->motion_props)) { return; }

	// particles
	p_emitter->position = rb->position;
	p_emitter->orientation = rb->orientation;

	elapsed_time += dt;

	if (elapsed_time < apex_threshold) {
		// Hover animation
		rb->position.y = starting_height + (rising_speed * EasingFunction::EaseOutBounce(elapsed_time / apex_threshold));
	}
	else {
		// Follow player
		RigidBody* player_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();
		if (!player_rb) { return; }
		Vec3 moveVec = glm::normalize(player_rb->position - rb->position) * chase_speed * (elapsed_time / apex_threshold);
		rb->motion_props->linear_velocity = moveVec;
	}
}

void Collectable::SetTrailColor(Vec3 const& color) {
	p_emitter->color_gen_bnds = {
		.min = Vec4{ color, 1.0f },
		.max = Vec4{ color, 1.0f }
	};
}

BEGIN_ATTRIBUTES_FOR(Collectable)
DEFINE_MEMBER(Float32, rising_speed)
DEFINE_MEMBER(Float32, chase_speed)
DEFINE_MEMBER(Float32, apex_threshold)
DEFINE_MEMBER(Float32, starting_height)
END_ATTRIBUTES

