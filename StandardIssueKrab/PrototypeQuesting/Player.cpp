#include "stdafx.h"

#include "Player.h"

#include "Engine/InputAction.h"
#include "Engine/MemoryManager.h"
#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Enemy.h"
#include "Health.h"

Player::Player() :
	alloc{ p_memory_manager->GetCurrentAllocator() },
	player_action{ alloc.new_object<InputAction>("CombatPlayerControls.json") },
	forward_force{ 0.0f },
	right_force{ 0.0f },
	move_angle{ 0.0f },
	angular_speed{ 40.0f },
	speed{ 2000.0f },
	brake_speed{ 5000.0f },
	local_forward{ 0.0f },
	local_right{ 0.0f }
{

}

Player::~Player() noexcept {
	alloc.delete_object(player_action);
}

void Player::OnCollide(GameObject* other) {
	Enemy* p_enemy = other->HasComponent<Enemy>();
	if (p_enemy != nullptr) {
		Health* p_health = GetOwner()->HasComponent<Health>();
		p_health->TakeDamage(1);
	}
}

void Player::Update(Float32 dt) {
	GameObject* owner = GetOwner();
	RigidBody* p_rigidbody = owner->HasComponent<RigidBody>();
	MotionProperties* p_motion_props = p_rigidbody->motion_props;
	Transform* p_transform = owner->HasComponent<Transform>();

	local_forward = ForwardVector();
	local_right = Vec3{ -local_forward.z, 0.0f, local_forward.x };

	// Velocity in local ref frame
	Float32 vel_right = glm::dot(p_motion_props->linear_velocity, local_right);
	Float32 vel_forward = glm::dot(p_motion_props->linear_velocity, local_forward);

	// Resistance (drag, rolling resistance, friction) in local ref frame
	Float32 resistance_long = p_motion_props->linear_damping * vel_forward * std::abs(vel_forward) +
		(30.0f * p_motion_props->linear_damping) * vel_forward;
	Float32 resistance_lat = p_motion_props->linear_damping * vel_right * std::abs(vel_right) +
		(30.0f * p_motion_props->linear_damping) * vel_right;
	resistance_lat += 0.95f * p_motion_props->mass * 10.0f * vel_right;

	// Calculate forces
	forward_force = -resistance_long;
	right_force = -resistance_lat;

	Float32 car_speed = std::sqrtf(vel_forward * vel_forward + vel_right * vel_right);

	// Controlling forward movement
	if (player_action->IsActionPressed(InputAction::Actions::UP)) {
		forward_force += speed;
	}

	// Controlling backward movement
	if (player_action->IsActionPressed(InputAction::Actions::DOWN)) {
		forward_force -= brake_speed;
	}

	// Controlling Left movement
	if (player_action->IsActionPressed(InputAction::Actions::LEFT)) {
		Float32 steering_angle = 0.0f;
		if (car_speed > 1.01f) {
			steering_angle = glm::radians(angular_speed * 12.0f / car_speed);
			move_angle += (car_speed * std::sinf(steering_angle) / 4.0f) * dt;
			if (move_angle > 3.142f) {
				move_angle -= (2.0f * 3.142f);
			}
			p_transform->orientation = glm::toQuat(glm::rotate(Mat4{ 1.0f },
													move_angle,
													Vec3{ 0.0f, 1.0f, 0.0f }));
		}
	}

	// Controlling Right movement
	if (player_action->IsActionPressed(InputAction::Actions::RIGHT)) {
		Float32 steering_angle = 0.0f;
		if (car_speed > 1.01f) {
			steering_angle = glm::radians(angular_speed * 12.0f / car_speed);
			move_angle -= (car_speed * std::sinf(steering_angle) / 4.0f) * dt;
			if (move_angle < -3.142f) {
				move_angle += (2.0f * 3.142f);
			}
			p_transform->orientation = glm::toQuat(glm::rotate(Mat4{ 1.0f },
													move_angle,
													Vec3{ 0.0f, 1.0f, 0.0f }));
		}
	}

	p_rigidbody->AddForce(forward_force * local_forward + right_force * local_right);
}

Vec3 Player::ForwardVector() {
	return glm::rotate(Mat4{ 1.0f }, move_angle, Vec3{ 0.0f, 1.0f, 0.0f })
			* Vec4{ 0.0f, 0.0f, -1.0f, 0.0f };
}

BEGIN_ATTRIBUTES_FOR(Player)
END_ATTRIBUTES