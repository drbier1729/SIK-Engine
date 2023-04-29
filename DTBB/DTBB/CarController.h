#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration
class InputAction;
struct ParticleEmitter;
struct ConeLocalLight;

class CarController : public Component {
public:
	VALID_COMPONENT(CarController);
	ALLOW_PRIVATE_REFLECTION;

	CarController();
	~CarController();

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;

	void OnCollide(GameObject* other) override;
	void Link() override;

	void Disable() override;
	void Enable() override;

	void Update(Float32 dt);
	void FixedUpdate(Float32 dt);

	void AudioUpdate();

	Bool GrowChain() const;
	Bool ShrinkChain() const;

	void SetHeadlights(Bool headlights_on);

	InputAction& GetActionMap();

	Float32 GetCurrentCarSpeed() const;

	Bool IsDrifting() const;

	inline void SetAngle(Float32 angle) { 	
		move_angle = glm::radians(angle);
	}

private:
	void SetDrifting(Bool _is_drifting);
	void UpdateHeadlightPositions();

private:
	PolymorphicAllocator alloc;

	InputAction* player_action;
	//To determine if this controller is for player 1 or 2
	Uint8 player_controller_num;

	Float32 forward_force;
	Float32 right_force;

	Float32 move_angle;

	Float32 angular_speed;
	Float32 speed;
	Float32 car_speed;
	Float32 reverse_speed;

	Vec3 local_forward;
	Vec3 local_right;

	Bool is_drifting;

	ParticleEmitter* p_drift_emitter_1;
	ParticleEmitter* p_drift_emitter_2;

	ConeLocalLight* p_headlights[2];
	Float32 headlights_forward_offset;
	Float32 headlights_vertical_offset;
	Float32 headlights_width;
	Float32 headlights_radius;
	Float32 headlights_length;

	Float32 rev_vol;
	Float32 rev_increment;
	Float32 max_rev_vol;

	Int32 skid_channel_id;
	Int32 rev_channel_id;

	Bool rev_start;
	Bool revving;
	Bool fade_out_rev_sound;

	Bool is_drifting_last_frame; 
	Bool is_drifting_this_frame;
	Bool fade_out_rev;

	Bool enabled;
};