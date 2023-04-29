#include "stdafx.h"

#include "CarController.h"

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

CarController::CarController():
	alloc{ p_memory_manager->GetCurrentAllocator() },
	player_action{ nullptr },
	forward_force{ 0.0f },
	right_force{ 0.0f },
	move_angle{ 0.0f },
	angular_speed{ 40.0f },
	speed{ 2000.0f },
	car_speed { 0.0f },
	reverse_speed{ 1000.0f },
	local_forward{ 0.0f },
	local_right{ 0.0f },
	is_drifting{ false },
	is_drifting_last_frame{ false },
	is_drifting_this_frame{ false },
	fade_out_rev{ false },
	p_drift_emitter_1{ p_particle_system->NewEmitter() },
	p_drift_emitter_2{ p_particle_system->NewEmitter() },
	p_headlights{ p_graphics_manager->AddConeLocalLight(), p_graphics_manager->AddConeLocalLight() },
	headlights_forward_offset{ 3.0f },
	headlights_vertical_offset{ 0.0f },
	headlights_width{ 1.5f },
	headlights_radius{ 0.5f },
	headlights_length{ 1.0f },
	player_controller_num{ 1 },
	rev_channel_id{0},
	skid_channel_id{0},
	rev_start{true},
	revving{ false },
	fade_out_rev_sound{false},
	rev_vol{0}, rev_increment(1.0f), max_rev_vol(65.0f)
{
	// drift emitter settings
	p_drift_emitter_1->gravity_scale = 0.0f;
	p_drift_emitter_2->gravity_scale = 0.0f;
	p_drift_emitter_1->particles_per_sec = 50;
	p_drift_emitter_2->particles_per_sec = 50;
	p_drift_emitter_1->pos_gen_bnds = {
		.min = Vec3{ 0.0f, -0.2f, 0.0f },
		.max = Vec3{ 0.0f,  0.2f, 0.0f }
	};
	p_drift_emitter_2->pos_gen_bnds = {
		.min = Vec3{ 0.0f, -0.2f, 0.0f },
		.max = Vec3{ 0.0f,  0.2f, 0.0f }
	};
	p_drift_emitter_1->color_gen_bnds = {
		.min = Vec4{ 0.32f, 0.31f, 0.28f, 1.0f },
		.max = Vec4{ 0.38f, 0.37f, 0.34f, 1.0f }
	};
	p_drift_emitter_2->color_gen_bnds = {
		.min = Vec4{ 0.32f, 0.31f, 0.28f, 1.0f },
		.max = Vec4{ 0.38f, 0.37f, 0.34f, 1.0f }
	};
	p_drift_emitter_1->theta = {
		.min = glm::radians(-190.0f),
		.max = glm::radians(-170.0f)
	};
	p_drift_emitter_2->theta = {
		.min = glm::radians(-190.0f),
		.max = glm::radians(-170.0f)
	};
	p_drift_emitter_1->phi = {
		.min = glm::radians(-10.0f),
		.max = glm::radians(-30.0f)
	};
	p_drift_emitter_2->phi = {
		.min = glm::radians(-10.0f),
		.max = glm::radians(-30.0f)
	};
	p_drift_emitter_1->lifetime_secs_gen_bnds = {
		.min = 0.1f,
		.max = 0.4f
	};
	p_drift_emitter_2->lifetime_secs_gen_bnds = {
		.min = 0.1f,
		.max = 0.4f
	};
	p_drift_emitter_1->uniform_scale_gen_bnds = {
		.min = 0.2f,
		.max = 0.5f
	};
	p_drift_emitter_2->uniform_scale_gen_bnds = {
		.min = 0.2f,
		.max = 0.5f
	};

	// Headlights settings
	p_headlights[0]->color = Vec3(1.0f, 1.0f, 173.0f / 255.0f) * 3.0f;
	p_headlights[1]->color = Vec3(1.0f, 1.0f, 173.0f / 255.0f) * 3.0f;
}

CarController::~CarController() {
	p_particle_system->EraseEmitter(p_drift_emitter_2);
	p_particle_system->EraseEmitter(p_drift_emitter_1);
	
	p_graphics_manager->RemoveConeLocalLight(p_headlights[0]);
	p_graphics_manager->RemoveConeLocalLight(p_headlights[1]);
	
	if (player_action != nullptr)
		alloc.delete_object(player_action);
}

void CarController::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<CarController>(json_value, this);

	const char* action_map_string = json_value.FindMember("action_map_string")->value.GetString();
	player_action = alloc.new_object<InputAction>(action_map_string, player_controller_num);

	p_headlights[0]->radius = headlights_radius;
	p_headlights[1]->radius = headlights_radius;
	p_headlights[0]->length = headlights_length;
	p_headlights[1]->length = headlights_length;
}

void CarController::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<CarController>(json_value, this, alloc);
}

void CarController::Modify(rapidjson::Value const& json_value) {
	if (player_action != nullptr)
		alloc.delete_object(player_action);

	Deserialize(json_value);
}

void CarController::OnCollide(GameObject* other)
{
}

void CarController::Link() {
	Behaviour* p_behaviour = GetOwner()->HasComponent<Behaviour>();

	for (auto& script : p_behaviour->scripts) {
		script.script_state.set_function("SetDrifting", &CarController::SetDrifting, this);
		p_scripting_manager->RegisterActionMap(script.script_state, player_action);
	}

	player_action->SetPlayerControllerNum(player_controller_num);

	SetHeadlights(true);
}

void CarController::Update(Float32 dt) {
	if (not enabled)
		return;
	UpdateHeadlightPositions();
	is_drifting_this_frame = is_drifting;
	AudioUpdate();
}

void CarController::FixedUpdate(Float32 dt) {
	if (not enabled)
		return;

	GameObject* owner = GetOwner();
	RigidBody* p_rigidbody = owner->HasComponent<RigidBody>();
	MotionProperties* p_motion_props = p_rigidbody->motion_props;
	if (!(owner && p_rigidbody && p_motion_props)) { return; }
	Transform* p_transform = owner->HasComponent<Transform>();

	// local vectors
	local_forward = glm::rotate(Mat4{ 1.0f }, move_angle, Vec3{ 0.0f, 1.0f, 0.0f })
					* Vec4 { 0.0f, 0.0f, -1.0f, 0.0f };
	local_right = Vec3{ -local_forward.z, 0.0f, local_forward.x };

	// Velocity in local ref frame
	Float32 vel_right = glm::dot(p_motion_props->linear_velocity, local_right);
	Float32 vel_forward = glm::dot(p_motion_props->linear_velocity, local_forward);
	
	car_speed = std::sqrtf(vel_forward * vel_forward + vel_right * vel_right);

	// Resistance (drag, rolling resistance, friction) in local ref frame
	Float32 resistance_long = p_motion_props->linear_damping * vel_forward * std::fabsf(vel_forward) +
		(30.0f * p_motion_props->linear_damping) * vel_forward;
	Float32 resistance_lat = p_motion_props->linear_damping * vel_right * std::fabsf(vel_right) +
		(30.0f * p_motion_props->linear_damping) * vel_right;
	resistance_lat += 0.95f * p_motion_props->mass * 10.0f * vel_right;

	// Calculate forces
	forward_force = -resistance_long;
	right_force = -resistance_lat;

	// Drifting
	if (is_drifting) {
		forward_force *= 0.8f;
		right_force *= 0.1f;

		if (car_speed > 5.0f) {
			// particle effect
			p_drift_emitter_1->is_active = true;
			p_drift_emitter_2->is_active = true;

			p_drift_emitter_1->orientation = p_transform->orientation;
			p_drift_emitter_2->orientation = p_transform->orientation;

			// moving forward
			if (vel_forward > 1.0f) {
				p_drift_emitter_1->theta = {
					.min = glm::radians(-190.0f),
					.max = glm::radians(-170.0f)
				};
				p_drift_emitter_2->theta = {
					.min = glm::radians(-190.0f),
					.max = glm::radians(-170.0f)
				};
			}
			// reversing
			else if (vel_forward < 1.0f) {
				p_drift_emitter_1->theta = {
					.min = glm::radians(-10.0f),
					.max = glm::radians(10.0f)
				};
				p_drift_emitter_2->theta = {
					.min = glm::radians(-10.0f),
					.max = glm::radians(10.0f)
				};
			}

			// speed of particles
			p_drift_emitter_1->speed_gen_bnds = {
				.min = (car_speed / 2.0f) - 1.5f,
				.max = (car_speed / 2.0f) + 1.5f
			};
			p_drift_emitter_2->speed_gen_bnds = {
				.min = (car_speed / 2.0f) - 1.5f,
				.max = (car_speed / 2.0f) + 1.5f
			};

			Vec3 particle_direction = glm::normalize(-local_forward);

			// shift particle position to rear axle
			Vec3 particle_pos_offset = (particle_direction * p_transform->scale.z * 2.0f);
			// shift particle 1 position to one wheel
			p_drift_emitter_1->position = p_transform->position + particle_pos_offset - (glm::normalize(local_right) * p_transform->scale.x);
			// shift particle 2 position to other wheel
			p_drift_emitter_2->position = p_transform->position + particle_pos_offset + (glm::normalize(local_right) * p_transform->scale.x);

		}
		else {
			p_drift_emitter_1->is_active = false;
			p_drift_emitter_2->is_active = false;
		}
	}
	else {
		p_drift_emitter_1->is_active = false;
		p_drift_emitter_2->is_active = false;
	}

	// Controlling forward movement
	if (player_action->IsActionPressed(InputAction::Actions::UP)) {
				forward_force += speed;
	}

	// Controlling backward movement
	if (player_action->IsActionPressed(InputAction::Actions::DOWN)) {
		// braking
		if (vel_forward > 0.0f) {
			forward_force -= speed;
		}
		// reversing
		else {
			forward_force -= reverse_speed;
		}
	}

	// Controlling Left and Right movement
	Bool press_left{ player_action->IsActionPressed(InputAction::Actions::LEFT) };
	Bool press_right{ player_action->IsActionPressed(InputAction::Actions::RIGHT) };

	if (press_left || press_right) {
		// turn car
		if (car_speed > 1.5f) {
			Float32 steering_angle{ glm::radians(angular_speed * 12.0f / car_speed) };
			Float32 angle_change{ (car_speed * std::sinf(steering_angle) / 4.0f) * dt };
			// copysignf(...) changes the sign of the angle if car is moving backwards
			angle_change =  std::copysignf(angle_change, vel_forward);
			if (press_left) {
				move_angle += angle_change;
			}
			if (press_right) {
				move_angle -= angle_change;
			}
		}
	}
	p_rigidbody->orientation = Quat(Vec3{ 0.0f, move_angle, 0.0f });

	p_rigidbody->AddForce(forward_force * local_forward + right_force * local_right);
}

void CarController::UpdateHeadlightPositions() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();

	p_headlights[0]->position = p_tr->LocalToWorld(Vec3(0.5 * headlights_width, headlights_vertical_offset, -(headlights_forward_offset + (p_headlights[0]->length / 2))));
	p_headlights[1]->position = p_tr->LocalToWorld(Vec3(-0.5 * headlights_width, headlights_vertical_offset, -(headlights_forward_offset + (p_headlights[0]->length / 2))));

	p_headlights[0]->orientation = p_headlights[1]->orientation = p_tr->orientation;
}

Bool CarController::GrowChain() const
{
	return player_action->IsActionPressed(InputAction::Actions::ACTION_R1);
}

Bool CarController::ShrinkChain() const
{
	return player_action->IsActionPressed(InputAction::Actions::ACTION_L1);
}

InputAction& CarController::GetActionMap() {
	return *player_action;
}

Float32 CarController::GetCurrentCarSpeed() const {
	return car_speed;
}

Bool CarController::IsDrifting() const {
	return is_drifting;
}

void CarController::Disable() {
	enabled = false;
	SetHeadlights(false);
	p_drift_emitter_1->is_active = false;
	p_drift_emitter_2->is_active = false;

	p_audio_manager->Stop(rev_channel_id);
	rev_channel_id = 0;
}

void CarController::Enable() {
	enabled = true;
	SetHeadlights(true);
}

void CarController::SetHeadlights(Bool headlights_on) {
	p_headlights[0]->enabled = headlights_on;
	p_headlights[1]->enabled = headlights_on;
}

void CarController::AudioUpdate() {
	if (is_drifting_last_frame == false && is_drifting_this_frame == true) {
 		skid_channel_id = p_audio_manager->PlayAudio("SKID"_sid, 
														p_audio_manager->sfx_chanel_group, 
														p_audio_manager->skid_vol, 
														1.5f, false, 0);
	}

	if (is_drifting_last_frame == true && is_drifting_this_frame == false) {
		p_audio_manager->Stop(skid_channel_id);
	}

	is_drifting_last_frame = is_drifting_this_frame;
	is_drifting_this_frame = false;

	if (player_action->IsActionTriggered(InputAction::Actions::UP)) {
		rev_channel_id = p_audio_manager->PlayAudio("REVVING"_sid,
			p_audio_manager->sfx_chanel_group,
			max_rev_vol,
			1.0f,
			false,
			-1);
	}

	if (not player_action->IsActionPressed(InputAction::Actions::UP)) {
		p_audio_manager->Stop(rev_channel_id);
		rev_channel_id = 0;
	}
}

void CarController::SetDrifting(Bool _is_drifting) {
	is_drifting = _is_drifting;
}

BEGIN_ATTRIBUTES_FOR(CarController)
	DEFINE_MEMBER(Float32, speed)
	DEFINE_MEMBER(Float32, reverse_speed)
	DEFINE_MEMBER(Float32, angular_speed)
	DEFINE_MEMBER(Float32, headlights_forward_offset)
	DEFINE_MEMBER(Float32, headlights_vertical_offset)
	DEFINE_MEMBER(Float32, headlights_width)
	DEFINE_MEMBER(Float32, headlights_radius)
	DEFINE_MEMBER(Float32, headlights_length)
	DEFINE_MEMBER(Uint8, player_controller_num)
END_ATTRIBUTES