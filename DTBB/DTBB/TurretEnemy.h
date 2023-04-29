#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declarations
class GameObject;
struct ParticleEmitter;
struct LocalLight;

class TurretEnemy : public Component {
public:
	VALID_COMPONENT(TurretEnemy);
	ALLOW_PRIVATE_REFLECTION;

	TurretEnemy();
	~TurretEnemy() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Link() override;

	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);

	Bool IsAlive() const;
	Bool IsDying() const;
	void SetDeathTimer(Float32 when_secs = 0.0f);

	void Disable() override;
	void Enable() override;
	void Reset() override;

private:
	void SetAggro(Bool _is_aggro);
	void SayGoodbyeAndDie();
	void SetupParticleEmitters();

private:
	GameObject* p_target;
	ParticleEmitter* p_turret_emitter;
	ParticleEmitter* p_hit_emitter;
	ParticleEmitter* p_death_emitter;
	ParticleEmitter* p_spark_emitter;
	ParticleEmitter* p_smoke_emitter;
	ParticleEmitter* p_flame_emitter;

	Bool is_alive;
	Bool is_aggro;
	Bool collided_last_frame;
	Bool collided_this_frame;

	Float32 dead_particles_timer;
	Float32 spark_particles_timer;
	Float32 invulnerability_timer;
	Float32 death_timer;

	LocalLight* p_light;

	// Serializable members
	Float32 aggro_radius;
	const char* target_name;
	Int32 bullet_damage;
	Float32 damage_momentum;
	Float32 secs_per_bullet;
	Vec3 light_col;
	Float32 explosion_radius;
};