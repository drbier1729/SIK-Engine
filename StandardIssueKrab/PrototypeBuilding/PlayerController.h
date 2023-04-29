#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration
class InputAction;

class PlayerController : public Component {
public:
	VALID_COMPONENT(PlayerController);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(PlayerController);

	PlayerController();
	~PlayerController() noexcept;

	void Update(Float32 dt);

	/*
	* Gets the current Speed of the player
	* Returns Float32 - forward_force
	*/
	Float32 GetForce() const;

	/*
	* Checks if the controller has reached top speed
	* (Within a small range)
	* Returns: Bool - True if reached top speed;
	*/
	Bool IsTopSpeed() const;

	/*
	* Returns the vector which points infront of the player
	* Returns: Vec3 - Forward Vector
	*/
	Vec3 ForwardVector();
private:
	InputAction* player_action;

	Float32 forward_force;
	Float32 max_force;

	Float32 orientation_angle;

	Float32 angular_speed;
	Float32 speed;
	Float32 reverse_speed;
};