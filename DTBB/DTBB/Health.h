#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

class Health : public Component {
public:
	VALID_COMPONENT(Health);
	ALLOW_PRIVATE_REFLECTION;

	Health();
	~Health() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Link() override;
	void Reset() override;

	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);

	void TakeDamage(Int32 damage);
	Int32 GetCurrHP() const;
	Int32 GetMaxHP() const;
	void ToggleInvincibility();

	Bool TookDamageThisFrame() const;
	Bool TookDamageLastFrame() const;

private:
	// serializable members
	Int32 hp_max;
	Int32 hp_curr;
	bool took_dmg_this_frame;
	bool took_dmg_last_frame;
	bool invincible;
};