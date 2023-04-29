#pragma once
#include "Engine/InputAction.h"
#include "Engine/GameObject.h"

class Player {
public:
	Player();
	~Player();

	void Update(float dt);
	Vec3 NewForwardVector();
	Vec3 GetRightVector();
	void AddComponent(Component* comp);
	void ResetPlayer();
	GameObject* GetGameObject();

private:
	GameObject* player_object;
	InputAction* action_map;
	InputAction* action_map_wasd;
	RigidBody* player_rigidbody;
	Transform* player_transform;
	MotionProperties* player_motion_properties;

	float forward_force;
	float right_force;
	float move_angle;
	float angular_speed;
	float total_distance;
	float speed;
	float break_speed;
	float old_distance;
		
	glm::vec3 local_forward_direction;
};

