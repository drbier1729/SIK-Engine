#pragma once
#include "stdafx.h"
#include "Player.h"
#include "Engine/PrototypeInterface.h"
#include "Engine/Factory.h"
#include "Engine/GraphicsManager.h"
#include "Engine/RigidBody.h"
#include "Engine/MotionProperties.h"
#include "Engine/InputManager.h"
#include "Engine/GUIObject.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"
#include "CountdownTimer.h"
#include "Engine/AudioManager.h"
#include "Engine/FrameTimer.h"

extern CountdownTimer* countdown_timer;

Player::Player() : forward_force(0.0f),
					right_force(0.0f),
					move_angle(0.0f),
					angular_speed(40.0f),
					total_distance(0.0f),
					old_distance(0.0f),
					speed(2000.0f),
					break_speed(2500.0f) {
	player_object = p_factory->BuildGameObject("PlayerObj.json");

	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();
	action_map = alloc.new_object<InputAction>("prototype1_character_controls.json");
	action_map_wasd = alloc.new_object<InputAction>("prototype1_character_controls_wasd.json");

	player_rigidbody = player_object->HasComponent<RigidBody>();
	player_transform = player_object->HasComponent<Transform>();
	player_motion_properties = player_rigidbody->motion_props;
}

Player::~Player() {
	p_memory_manager->GetCurrentAllocator().delete_object(action_map);
	p_memory_manager->GetCurrentAllocator().delete_object(action_map_wasd);
}

void Player::Update(float dt) {
	//local_forward_direction = ComputeForwardVector(player_transform->orientation);
	local_forward_direction = NewForwardVector();

	// Velocity in local ref frame
	float vel_right = glm::dot(player_motion_properties->linear_velocity, GetRightVector());
	float vel_forward = glm::dot(player_motion_properties->linear_velocity, local_forward_direction);

	// Resistance (drag, rolling resistance, friction) in local ref frame
	float resistance_long = player_motion_properties->linear_damping * vel_forward * abs(vel_forward) +
		(30.0f * player_motion_properties->linear_damping) * vel_forward;
	float resistance_lat = player_motion_properties->linear_damping * vel_right * abs(vel_right) +
		(30.0f * player_motion_properties->linear_damping) * vel_right;


	resistance_lat += 0.95f * player_motion_properties->mass * 10.0f * vel_right;

	// Calculate forces
	forward_force = -resistance_long;
	right_force = -resistance_lat;

	float car_speed = std::sqrtf(vel_forward * vel_forward + vel_right * vel_right);

	int temp = static_cast<int>(old_distance);
	std::string string_text = std::to_string(temp);
	const char* display_text = string_text.c_str();

	p_gui_object_manager->GetTextObject(2)->SetText(display_text);

	if (countdown_timer->GetTimeState())
	{
		old_distance = total_distance;
		total_distance += glm::distance(Vec3(player_motion_properties->prev_position.x, 0.0, player_motion_properties->prev_position.z),
			Vec3(player_rigidbody->position.x, 0.0, player_rigidbody->position.z));


		// Controlling forward movement
		if (action_map->IsActionPressed(InputAction::Actions::UP) ||
			action_map_wasd->IsActionPressed(InputAction::Actions::UP)) {
			forward_force += speed;
		}


		// Controlling backward movement
		if (action_map->IsActionPressed(InputAction::Actions::DOWN) ||
			action_map_wasd->IsActionPressed(InputAction::Actions::DOWN)) {
			forward_force -= break_speed;
		}


		// Controlling Left movement
		if (action_map->IsActionPressed(InputAction::Actions::LEFT) ||
			action_map_wasd->IsActionPressed(InputAction::Actions::LEFT)) {
			/*forward_force += speed * dt;
			player_rigidbody->motion_props->AddForce((forward_force * -(GetRightVector())));*/
			float steering_angle = 0.0f;
			if (car_speed > 1.01) {
				steering_angle = glm::radians(angular_speed) * 12.0f / car_speed;
				move_angle += (car_speed * std::sinf(steering_angle) / 4.0f) * dt;
				if (move_angle > 3.142f)
					move_angle = move_angle - (2 * 3.142f);

				player_transform->orientation = glm::toQuat(glm::rotate(Mat4(1),
					move_angle,
					Vec3(0.0f, 1.0f, 0.0f)));
			}
		}

		// Controlling Right movement
		if (action_map->IsActionPressed(InputAction::Actions::RIGHT) ||
			action_map_wasd->IsActionPressed(InputAction::Actions::RIGHT)) {

			float steering_angle = 0.0f;
			if (car_speed > 1.01) {
				steering_angle = glm::radians(angular_speed) * 12.0f / car_speed;
				move_angle -= (car_speed * std::sinf(steering_angle) / 4.0f) * dt;
				if (move_angle < -3.142f)
					move_angle = move_angle + (2 * 3.142f);

				player_transform->orientation = glm::toQuat(glm::rotate(Mat4(1),
					move_angle,
					Vec3(0.0f, 1.0f, 0.0f)));
			}
		}
	}

	player_rigidbody->AddForce(forward_force * local_forward_direction + right_force * GetRightVector());


	if (glm::distance(Vec3(0.0, player_rigidbody->position.y, 0.0), Vec3(0.0)) < 5.0 && 
		glm::distance(Vec3(0.0, player_rigidbody->position.y, 0.0), Vec3(0.0)) > 4.5)
	{
		p_audio_manager->PlaySoundEffect("FALLEN");
	}

	if (glm::distance(Vec3(0.0, player_rigidbody->position.y, 0.0), Vec3(0.0)) == 0.0)
	{
		ResetPlayer();
		countdown_timer->ResetTimer();
	}
}

Vec3 Player::NewForwardVector()
{
	glm::vec3 temp = glm::rotate(Mat4(1.0f), move_angle, Vec3(0.0f, 1.0f, 0.0f)) * Vec4(0.0f, 0.0f, -1.0f, 0.0f);
	return temp;
}

Vec3 Player::GetRightVector()
{
	Vec3 temp = Vec3(-local_forward_direction.z, 0.0, local_forward_direction.x);
	return temp;
}

void Player::ResetPlayer()
{
	player_rigidbody->position = Vec3(0.0, 11.0, 0.0);
	player_motion_properties->prev_position = player_rigidbody->position;
	player_motion_properties->accumulated_force = Vec3(0.0f);
	player_motion_properties->angular_velocity = Vec3(0.0f);
	player_motion_properties->linear_velocity = Vec3(0.0f);
	player_transform->orientation = Vec3(0.0f);
	player_motion_properties->prev_orientation = player_rigidbody->orientation;
	forward_force= 0.0f;
	right_force = 0.0f;
	move_angle = 0.0f;
	total_distance = 0.0f;
}

void Player::AddComponent(Component* comp) {
	player_object->AddComponent(comp);
}

GameObject* Player::GetGameObject()
{
	return player_object;
}