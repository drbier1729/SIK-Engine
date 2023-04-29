#pragma once

#include "Engine/Component.h"

// forward declarations
struct ParticleEmitter;

/*
* Having a component specify the player character avoids having to iterate through all the 
* game objects searching for the player game object using a name string.
*/
class PlayerCharacter : public Component
{
public:
	VALID_COMPONENT(PlayerCharacter);
	
	PlayerCharacter();
	~PlayerCharacter() noexcept;

	void Deserialize(rapidjson::Value const& json_value) override;
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) override;
	void Modify(rapidjson::Value const& json_value) override;

	void Link() override;
	void Enable() override;
	void Disable() override;

	void Update(Float32 dt) override;

	void TakeDamage(Int32 bullet_damage);

private:
	ParticleEmitter* p_bullet_hit_emitter;
	ParticleEmitter* p_smoke_emitter;
	ParticleEmitter* p_flame_emitter;
};

