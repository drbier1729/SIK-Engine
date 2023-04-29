#pragma once

#include "Engine/Component.h"

// fwd decls
class GameObject;
class Turret;

class Enemy : public Component {
public:
	GameObject* player_obj;
	Turret* p_turret;

public:
	VALID_COMPONENT(Enemy);

	Enemy();
	~Enemy() noexcept = default;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);
};