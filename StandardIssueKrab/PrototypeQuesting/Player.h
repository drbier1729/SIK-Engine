#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration
class InputAction;

class Player : public Component {
public:
	VALID_COMPONENT(Player);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(Player);

	Player();
	~Player() noexcept;

	void OnCollide(GameObject* other) override;
	void Update(Float32 dt);

	Vec3 ForwardVector();

private:
	PolymorphicAllocator alloc;
	InputAction* player_action;

	Float32 forward_force;
	Float32 right_force;

	Float32 move_angle;

	Float32 angular_speed;
	Float32 speed;
	Float32 brake_speed;

	Vec3 local_forward;
	Vec3 local_right;
};