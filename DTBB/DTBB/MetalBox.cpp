#include "stdafx.h"

#include "MetalBox.h"

#include "Engine/InputAction.h"
#include "Engine/MemoryManager.h"
#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Engine/ParticleSystem.h"
#include "Engine/ResourceManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/AudioManager.h"
#include "Engine/InputManager.h"
#include "Collectable.h"
#include "BallnChain.h"
#include "BaseState.h"


MetalBox::MetalBox():
	collided_this_frame{false},
	collided_last_frame{ false },
	sound_cooldown_timer{ 3.0f },
	orig_pos{ 0.0f },
	orig_ori{}
{
}

MetalBox::~MetalBox()
{
}

void MetalBox::OnCollide(GameObject* other){
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (not rb->IsEnabled())
		return;

	if (other == p_base_state->GetPlayerGameObjPtr() || other->HasComponent<BallnChain>())
	{
		RigidBody* other_rb = other->HasComponent<RigidBody>();

		Vec3 owner_velocity = rb->motion_props->linear_velocity;
		Vec3 other_velocity = other_rb->motion_props->linear_velocity;
		Float32 relative_speed = glm::length(owner_velocity - other_velocity);

		if (relative_speed > 5.0f) {
			collided_this_frame = true;
		}
	}

}

void MetalBox::Link() {
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	orig_pos = p_rb->position;
	orig_ori = p_rb->orientation;
}

void MetalBox::Reset() {
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	p_rb->position    = orig_pos;
	p_rb->orientation = orig_ori;
}

void MetalBox::Update(Float32 dt)
{
	if (collided_last_frame == false && collided_this_frame == true) {// collision enter
		if (sound_cooldown_timer <= 0.01f) {
			sound_cooldown_timer = 3.0f;
			AudioUpdate();
		}
	}

	if (collided_last_frame == true && collided_this_frame == true) {// collision stay

	}

	if (collided_last_frame == true && collided_this_frame == false) {// collision exit

	}

	collided_last_frame = collided_this_frame;
	collided_this_frame = false;

	sound_cooldown_timer -= dt;
}

void MetalBox::AudioUpdate() {

	// Hit sound effect
	p_audio_manager->PlayAudio("TURRET_IMP"_sid,
		p_audio_manager->sfx_chanel_group,
		p_audio_manager->turret_impact_vol,
		1.5f,
		false,
		0);
}


BEGIN_ATTRIBUTES_FOR(MetalBox)
END_ATTRIBUTES