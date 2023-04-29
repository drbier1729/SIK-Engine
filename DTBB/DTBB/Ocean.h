#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

class Ocean : public Component {
public:
	VALID_COMPONENT(Ocean);
	ALLOW_PRIVATE_REFLECTION;

	Ocean();
	~Ocean() noexcept = default;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Link() override;
	void Reset() override;

	void Modify(rapidjson::Value const& json_value);

	void Update(Float32 dt);

private:
	Vec3 orig_pos;
	Quat orig_ori;

	Bool receding;
	Bool raising;

	// serializable members
	Float32 x_speed;
	Float32 y_speed;
};