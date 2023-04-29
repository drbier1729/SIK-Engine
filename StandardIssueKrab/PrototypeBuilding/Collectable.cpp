#include "stdafx.h"
#include "Collectable.h"
#include "Inventory.h"
#include "PlayerController.h"
#include "Engine/GameObject.h"

std::unordered_map<StringID, Collectable::CTypes> Collectable::string_types_map{
	{"Resource1"_sid, Collectable::CTypes::Resource1},
	{"Resource2"_sid, Collectable::CTypes::Resource2}
};

void Collectable::Deserialize(rapidjson::Value const& json_value) {
	auto res_iter = string_types_map.find(
		ToStringID(json_value.FindMember("Type")->value.GetString()));

	SIK_ASSERT(res_iter != string_types_map.end(), "Failed to find collectable type");

	c_type = res_iter->second;
}

void Collectable::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc)
{
}

void Collectable::OnCollide(GameObject* other) {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (not rb->IsEnabled())
		return;

	if (not other->HasComponent<PlayerController>())
		return;

	is_collected = true;
	GetOwner()->Disable();
	Inventory* obj_inv = other->HasComponent<Inventory>();
	if (obj_inv) {
		obj_inv->AddCollectable(this);
	}
}

void Collectable::Update(Float32 dt) {
	bool rb_active = GetOwner()->HasComponent<RigidBody>()->IsEnabled();
}


