#include "stdafx.h"

#include "PlayerController.h"

#include "Engine/InputAction.h"
#include "Engine/MemoryManager.h"
#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"

PlayerController::PlayerController() :
	player_action{ p_memory_manager->GetCurrentAllocator().new_object<InputAction>("CombatPlayerControls.json")},
	forward_force{ 0.0f },
	max_force { 2000.0f },
	orientation_angle{ 0.0f },
	angular_speed{ 80.0f },
	speed{ 500.0f },
	reverse_speed{ 1000.0f }
{

}

PlayerController::~PlayerController() noexcept {
	p_memory_manager->GetCurrentAllocator().delete_object(player_action);
}

void PlayerController::Update(Float32 dt) {
	GameObject* owner = GetOwner();
	RigidBody* p_rigidbody = owner->HasComponent<RigidBody>();
	MotionProperties* p_motion_props = p_rigidbody->motion_props;
	Transform* p_transform = owner->HasComponent<Transform>();

	Vec3 local_forward = ForwardVector();

	// Controlling forward movement
	if (player_action->IsActionPressed(InputAction::Actions::UP)) {
		forward_force += speed * dt;
		p_rigidbody->AddForce(forward_force * local_forward);
	}
	else {
		forward_force -= speed * dt;
	}

	// Controlling backward movement
	if (player_action->IsActionPressed(InputAction::Actions::DOWN)) {
		forward_force = -1 * reverse_speed * dt;
		p_rigidbody->AddForce(forward_force * local_forward);
	}

	// Controlling Left movement
	if (player_action->IsActionPressed(InputAction::Actions::LEFT)) {
		orientation_angle += glm::radians(angular_speed * dt);
	}

	// Controlling Right movement
	if (player_action->IsActionPressed(InputAction::Actions::RIGHT)) {
		orientation_angle -= glm::radians(angular_speed * dt);
	}

	//Clamp the move angle
	if (orientation_angle < -1 * glm::pi<Float32>())
		orientation_angle = glm::pi<Float32>();
	else if (orientation_angle > glm::pi<Float32>())
		orientation_angle = -1 * glm::pi<Float32>();

	//Clamp the forward force 
	forward_force = glm::max(glm::min(max_force, forward_force), 0.0f);

	p_transform->orientation =
		glm::toQuat(glm::rotate(Mat4{ 1.0f }, orientation_angle, Vec3{ 0.0f, 1.0f, 0.0f }));
}

/*
* Gets the current Speed of the player
* Returns Float32 - forward_force
*/
Float32 PlayerController::GetForce() const {
	return forward_force;
}

/*
* Checks if the controller has reached top speed
* (Within a small range)
* Returns: Bool - True if reached top speed;
*/
Bool PlayerController::IsTopSpeed() const {
	Float32 range = 200.0f;
	return (forward_force > (max_force - range));
}

Vec3 PlayerController::ForwardVector() {
	return glm::rotate(Mat4{ 1.0f }, orientation_angle, Vec3{ 0.0f, 1.0f, 0.0f })
		* Vec4 {
		0.0f, 0.0f, -1.0f, 0.0f
	};
}

BEGIN_ATTRIBUTES_FOR(PlayerController)
END_ATTRIBUTES