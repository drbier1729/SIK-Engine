#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// Forward Declaration
class GameObject;

class Attachment : public Component {
public:
	VALID_COMPONENT(Attachment);
	ALLOW_PRIVATE_REFLECTION;

	Attachment();
	~Attachment() noexcept = default;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Update(Float32 dt);

private:
	GameObject* p_attached_to;
	Bool follow;
	Vec3 pos_offset;
};