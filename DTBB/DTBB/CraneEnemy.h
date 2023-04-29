#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration

class CraneEnemy : public Component {
public:
	VALID_COMPONENT(CraneEnemy);
	ALLOW_PRIVATE_REFLECTION;

	CraneEnemy();
	~CraneEnemy() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;

	void OnCollide(GameObject* other) override;
	void Link() override;

	void Enable() override;
	void Disable() override;

	void Reset() override;

	void Update(Float32 dt);
	void FixedUpdate(Float32 dt);

private:
	Vec3 orig_pos;
	Quat orig_ori;
};