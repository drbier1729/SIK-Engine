#include "stdafx.h"

#include "Health.h"

#include "Engine/GameObject.h"
#include "CarController.h"
#include "Engine/InputManager.h"

Health::Health():
	hp_max{ 0 },
	hp_curr{ 0 }, took_dmg_this_frame(false), took_dmg_last_frame(false),
	invincible{false}
{
}

Health::~Health() noexcept {
}

void Health::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Health>(json_value, this);
}

void Health::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Health>(json_value, this, alloc);
}

void Health::Link() {
	hp_curr = hp_max;
}

void Health::Reset() {
	hp_curr = hp_max;

	took_dmg_last_frame = false;
	took_dmg_this_frame = false;
}

void Health::OnCollide(GameObject* other)
{
}

void Health::Update(Float32 dt) {
	if (took_dmg_last_frame) {
		took_dmg_last_frame = false;
	}

	if (took_dmg_this_frame) {
		took_dmg_this_frame = false;
		took_dmg_last_frame = true;
	}
}

void Health::TakeDamage(Int32 damage) {
	static constexpr auto max_health = 0x7FFF; // ~32000

	// clamp damage to prevent signed integer overflow UB
	damage = glm::clamp(damage, -(hp_max - hp_curr), (!invincible) * hp_curr);

	hp_curr -= damage;
	hp_curr = glm::clamp(hp_curr, 0, hp_max);

	CarController* player = GetOwner()->HasComponent<CarController>();
	if (player != nullptr){
		p_input_manager->RumbleController(500, 5000, 100);
	}

	took_dmg_this_frame = true;
}


void Health::ToggleInvincibility() {
	invincible = !invincible;
}

Int32 Health::GetCurrHP() const {
	return hp_curr;
}

Int32 Health::GetMaxHP() const {
	return hp_max;
}

Bool Health::TookDamageThisFrame() const {
	return took_dmg_this_frame;
}

Bool Health::TookDamageLastFrame() const {
	return took_dmg_last_frame;
}

BEGIN_ATTRIBUTES_FOR(Health)
DEFINE_MEMBER(Int32, hp_max)
END_ATTRIBUTES