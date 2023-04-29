#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration
class Magnet;

class Debris : public Component {
public:
	VALID_COMPONENT(Debris);
	ALLOW_PRIVATE_REFLECTION;

	Debris();
	~Debris() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;

	void Link() override;
	void Enable() override;
	void Disable() override;

	void Reset() override;

	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);
	void FixedUpdate(Float32 dt);

	void ShotFromMagnet(Vec3 new_vel);
	void SetStuck(Bool _is_stuck);

	void AudioUpdate();

	Bool IsStuck();

private:
	void ChaseTarget(Float32 dt);

private:
	Magnet* magnet;
	Bool is_stuck;

	Bool collided_last_frame;
	Bool collided_this_frame;

	// timer activated after shot, so it doesn't get pulled back to magnet
	Float32 timer;

	Vec3 orig_pos;
	Quat orig_ori;
	// original gravity scale
	Float32 gravity_scale;
	// original scale of body
	Vec3 transform_scale;
	// original mass
	Float32 original_mass;

	// serializable members
	Float32 chase_speed;
	Float32 max_chase_dist;
};