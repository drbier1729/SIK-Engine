#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"
#include "Engine/RandomGenerator.h"

// forward declaration
struct ParticleEmitter;
struct LocalLight;
struct ConeLocalLight;
struct Transform;
class GameObject;

class GenericCarEnemy : public Component {
public:
	VALID_COMPONENT(GenericCarEnemy);
	ALLOW_PRIVATE_REFLECTION;

	enum class EnemyType {
		SmallCar,
		BigCar
	};

	GenericCarEnemy();
	~GenericCarEnemy() noexcept;

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

	EnemyType GetEnemyType();

	void SetHeadlights(Bool headlights_on);

private:
	void SetAggro(Bool _is_aggro);
	void ChaseTarget(Vec3 const& final_dir, Float32 dt);
	Bool CloseEnough(Vec3 const& loc1, Vec3 const& loc2);
	void CheckSameSpot(Float32 dt);
	// update health of the owner
	// parameter: amount of damage taken
	void HealthUpdate(Int32 damage);
	void SetupEmitters();
	void UpdateHeadlightPositions();

private:
	static std::unordered_map<StringID, EnemyType> enemy_types_map;
	EnemyType enemy_type;

	GameObject* target_go;
	Transform* target_tr;

	Vec3 local_forward;
	Vec3 local_right;

	Float32 forward_force;
	Float32 right_force;

	Float32 move_angle;

	Bool is_alive;
	Bool is_chasing;
	Bool is_aggro;
	Bool collided_last_frame;
	Bool collided_this_frame;

	Bool player_collision;
	Bool self_damaging_collision;

	Vec3 orig_pos;
	Quat orig_ori;
	UniquePtr<RandomGenerator<Float32>> rand_float;

	ParticleEmitter* p_smoke_emitter;
	ParticleEmitter* p_flame_emitter;

	// Headlights
	ConeLocalLight* p_headlights[2];
	Float32 headlights_forward_offset;
	Float32 headlights_vertical_offset;
	Float32 headlights_width;
	Float32 headlights_radius;
	Float32 headlights_length;
	Vec3 headlights_color;

	// simple obstacle avoidance
	Bool in_same_spot;
	Float32 in_same_spot_timer;
	Vec3 prev_pos;
	Float32 reverse_timer;
	Int8 turn_direction;

	// serializable members
	Float32 max_forward_speed;
	Float32 max_reverse_speed;
	Float32 max_angular_speed;
	Float32 aggro_radius;
	Int32 damage;
	Float32 destroy_vel_threshold;	// min velocity of colliding body required to kill this enemy
};