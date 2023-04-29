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
	void Link() override;
	void Modify(rapidjson::Value const& json_value);

	void Enable() override;

	void OnCollide(GameObject* other) override;

	void FixedUpdate(Float32 dt);

	void SetAttachedTo(GameObject* p_attached_to);
	GameObject* GetAttachedTo();
	void SetFollow(Bool const& _follow);
	void SetLockOrientation(Bool const& _lock_ori);
	void SetOffset(Vec3 const& _offset);
	Vec3 const& GetOffset();

private:
	GameObject* p_go_attached_to;

	// serializable members
	String name_attached_to;
	Bool follow;
	Bool lock_orientation;
	Vec3 pos_offset;
};