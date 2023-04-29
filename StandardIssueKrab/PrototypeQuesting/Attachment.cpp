#include "stdafx.h"

#include "Attachment.h"

#include "Engine/GameObject.h"
#include "Engine/GameObjectManager.h"

Attachment::Attachment() :
	p_attached_to{ nullptr },
	follow{ true },
	pos_offset{ 0 } {

}

void Attachment::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Attachment>(json_value, this);

	auto it = json_value.FindMember("AttachedTo");
	SIK_ASSERT(it->value.IsString(), "Name must be string");
	String name = it->value.GetString();

	// find game object with matching name
	auto& game_objs = p_game_obj_manager->GetGameObjectContainer();
	for (auto range = game_objs.all(); !range.is_empty(); range.pop_front()) {
		GameObject& game_obj = range.front();
		if (game_obj.GetName() == name) {
			p_attached_to = &game_obj;
			break;
		}
	}
}

void Attachment::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Attachment>(json_value, this, alloc);

	auto it = json_value.FindMember("AttachedTo");
	it->value.SetString(p_attached_to->GetName().c_str(), alloc);
}

void Attachment::Update(Float32 dt) {
	GameObject* p_owner = GetOwner();

	Transform* p_owner_transform = p_owner->HasComponent<Transform>();
	Transform* p_attached_to_transform = p_attached_to->HasComponent<Transform>();

	// Set the position of owner GameObject relative to AttachedTo GameObject
	p_owner_transform->position = p_attached_to_transform->position + pos_offset;
	// Set the orientation of owner GameObject same as AttachedTo GameObject
	p_owner_transform->orientation = p_attached_to_transform->orientation;
}

BEGIN_ATTRIBUTES_FOR(Attachment)
	DEFINE_MEMBER(Bool, follow)
	DEFINE_MEMBER(Vec3, pos_offset)
END_ATTRIBUTES