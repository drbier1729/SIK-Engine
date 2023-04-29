#include "stdafx.h"

#include "Magnet.h"

#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Engine/ScriptingManager.h"
#include "Engine/AudioManager.h"

#include "Attachment.h"
#include "BaseState.h"
#include "Debris.h"

Magnet::Magnet():
	attached_debris{},
	num_attached_debris{ 0 },
	sound_id{ 0 },
	play_loaded_sound{ false },
	orig_pos{ 0.0f },
	orig_ori{},
	shoot_speed{ 0.0f }
{
	attached_debris.reserve(MAX_ATTACHED_DEBRIS);

	// initialize vector of attached debris

	// layer 1
	attached_debris.push_back(std::make_tuple(Vec3{  1.8f, -1.5f, -1.8f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{  1.8f, -1.5f,  0.25f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{  1.8f, -1.5f,  2.3f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{  0.0f, -1.5f,  2.3f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -1.8f, -1.5f,  2.3f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -1.8f, -1.5f,  0.25f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -1.8f, -1.5f, -1.8f }, static_cast<Debris*>(nullptr)));

	// layer 2
	attached_debris.push_back(std::make_tuple(Vec3{  1.8f, -0.5f, -0.775f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{  1.8f, -0.5f,  1.275f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{  0.9f, -0.5f,  2.3f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -0.9f, -0.5f,  2.3f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -1.8f, -0.5f,  1.275f }, static_cast<Debris*>(nullptr)));
	attached_debris.push_back(std::make_tuple(Vec3{ -1.8f, -0.5f, -0.775f }, static_cast<Debris*>(nullptr)));
}

Magnet::~Magnet() noexcept {
}

void Magnet::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Magnet>(json_value, this);
}

void Magnet::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Magnet>(json_value, this, alloc);
}

void Magnet::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void Magnet::OnCollide(GameObject* other) {
}

void Magnet::Link() {
	GameObject* player_obj = p_base_state->GetPlayerGameObjPtr();

	Attachment* p_att = GetOwner()->HasComponent<Attachment>();
	p_att->SetAttachedTo(player_obj);

	Behaviour* p_behaviour = player_obj->HasComponent<Behaviour>();
	for (auto& script : p_behaviour->scripts) {
		script.script_state.set_function("ShootDebris", &Magnet::ShootDebris, this);
	}

	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	orig_pos = p_rb->position;
	orig_ori = p_rb->orientation;
}

void Magnet::Enable() {
	Link();
}

void Magnet::Disable() {
	p_audio_manager->Stop(sound_id);
}

void Magnet::Reset() {
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	p_rb->position = orig_pos;
	p_rb->orientation = orig_ori;
}

void Magnet::Update(Float32 dt) {
	if (num_attached_debris > 0 && play_loaded_sound) {
		sound_id = p_audio_manager->PlayAudio("MAGLOAD"_sid,
			p_audio_manager->sfx_chanel_group,
			80.0f,
			1.0f,
			false,
			-1);

		play_loaded_sound = false;
	}
}

void Magnet::FixedUpdate(Float32 dt) {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	MotionProperties* mp = rb ? rb->motion_props : nullptr;
	SIK_ASSERT(rb != nullptr && mp != nullptr, "Magnet needs rigidbody");
	if (!rb || !mp) { return; }

	for (Uint8 i = 0; i < num_attached_debris; ++i) {
		Debris* debris = std::get<1>(attached_debris[i]);

		RigidBody* rb_debris = debris->GetOwner()->HasComponent<RigidBody>();
		MotionProperties* mp_debris = rb_debris ? rb_debris->motion_props : nullptr;
		if (rb_debris && mp_debris) {
			mp_debris->prev_position = rb_debris->position;
			mp_debris->linear_velocity = mp->linear_velocity;
			rb_debris->position = rb->LocalToWorld(std::get<0>(attached_debris[i]));
			rb_debris->orientation = rb->orientation;
		}
	}
}

void Magnet::AddDebris(Debris* p_debris) {
	if (HasFreeSlots()) {
		// modify ptr at current free index
		std::get<1>(attached_debris[num_attached_debris]) = p_debris;

		// set gravity scale of debris
		RigidBody* rb_debris = p_debris->GetOwner()->HasComponent<RigidBody>();
		MotionProperties* p_motion_props = rb_debris->motion_props;
		p_motion_props->gravity_scale = 0.0f;

		rb_debris->MakeTrigger(true);

		// increment attached debris count
		++num_attached_debris;

		// set stuck
		p_debris->SetStuck(true);

		// set scale
		Transform* debris_tr = p_debris->GetOwner()->HasComponent<Transform>();
		debris_tr->scale = Vec3{ 0.5f };
	}
	else {
		p_debris->SetStuck(false);
	}
}

Bool Magnet::HasFreeSlots() {
	return num_attached_debris < MAX_ATTACHED_DEBRIS;
}

// called from magnet script
void Magnet::ShootDebris() {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	SIK_ASSERT(rb != nullptr, "Magnet needs rigidbody");
	Vec3 magnet_pos = rb->position;
	magnet_pos.y -= 1.0f;

	if (num_attached_debris > 0) {
		p_audio_manager->Stop(sound_id);
		play_loaded_sound = true;
	}
	
	for (Uint16 i = 0; i < num_attached_debris; ++i) {
		Vec3 direction = rb->LocalToWorld(std::get<0>(attached_debris[i])) - magnet_pos;
		Debris* debris = std::get<1>(attached_debris[i]);

		// calculate world vector for new velocity of debris
		Vec3 new_vel = glm::normalize(direction) * shoot_speed;

		p_audio_manager->PlayAudio("MAGSHOOT"_sid,
			p_audio_manager->sfx_chanel_group,
			60.0f,
			1.0f,
			false,
			0);

		debris->ShotFromMagnet(new_vel);

		// remove debris
		std::get<1>(attached_debris[i]) = nullptr;

		// set unstuck
		debris->SetStuck(false);
	}

	num_attached_debris = 0;
}

Int32 Magnet::GetSoundID() {
	return sound_id;
}

void Magnet::SetNumAttached(Uint16 _num_attached_debris) {
	num_attached_debris = _num_attached_debris;
}

void Magnet::PlayLoadedSound(Bool _play_loaded_sound) {
	play_loaded_sound = _play_loaded_sound;
}

BEGIN_ATTRIBUTES_FOR(Magnet)
	DEFINE_MEMBER(Float32, shoot_speed)
END_ATTRIBUTES