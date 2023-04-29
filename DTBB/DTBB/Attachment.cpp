#include "stdafx.h"

#include "Attachment.h"

#include "Engine/GameObjectManager.h"
#include "Engine/GameStateManager.h"
#include "Engine/MotionProperties.h"

#include "ObjectHolder.h"

Attachment::Attachment():
	p_go_attached_to{ nullptr },
	name_attached_to{},
	follow{ true },
	lock_orientation{ true },
	pos_offset{ 0.0f }
{
}

void Attachment::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Attachment>(json_value, this);

	auto it = json_value.FindMember("attached_to");
	SIK_ASSERT(it->value.IsString(), "Name must be string");
	name_attached_to = it->value.GetString();
}

void Attachment::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Attachment>(json_value, this, alloc);

	auto it = json_value.FindMember("attached_to");
	it->value.SetString(name_attached_to.c_str(), alloc);
}

void Attachment::Link() {
	// find game object with matching name
	auto& game_objs = p_game_obj_manager->GetGameObjectContainer();
	for (auto range = game_objs.all(); !range.is_empty(); range.pop_front()) {
		GameObject& game_obj = range.front();
		if (game_obj.GetName().compare(name_attached_to) == 0) {
			p_go_attached_to = &game_obj;

			// add this object to holder's list
			ObjectHolder* p_obj_hold = p_go_attached_to->HasComponent<ObjectHolder>();
			SIK_ASSERT(p_obj_hold != nullptr, "ObjectHolder does not exist");
			p_obj_hold->AddAttachment(GetOwner());

			break;
		}
	}
}

void Attachment::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void Attachment::Enable() {
	
}

void Attachment::OnCollide(GameObject* other)
{
}

void Attachment::FixedUpdate(Float32 dt) {
	GameObject* p_owner = GetOwner();

	RigidBody* p_owner_rb = p_owner->HasComponent<RigidBody>();
	Transform* p_owner_transform = p_owner->HasComponent<Transform>();
	MotionProperties* mp = p_owner_rb ? p_owner_rb->motion_props : nullptr;

	Transform* p_attached_to_transform = p_go_attached_to->HasComponent<Transform>();
	RigidBody* p_attached_to_rb = p_go_attached_to->HasComponent<RigidBody>();
	MotionProperties* attached_to_mp = p_attached_to_rb ? p_attached_to_rb->motion_props : nullptr;

	// Set the position of owner GameObject relative to AttachedTo GameObject
	if (follow) {
		// if owner has RigidBody, modify that
		if (p_owner_rb && mp && attached_to_mp) {
			mp->prev_position = p_owner_rb->position;
			mp->linear_velocity = attached_to_mp->linear_velocity;
			p_owner_rb->position = p_attached_to_rb->LocalToWorld(pos_offset);
		}
		else {
			p_owner_transform->position = p_attached_to_transform->LocalToWorld(pos_offset);
		}
	}

	// Set the orientation of owner GameObject same as AttachedTo GameObject
	if (lock_orientation) {
		// if owner has RigidBody, modify that
		if (p_owner_rb && mp && attached_to_mp) {
			/*if (mp) {
				mp->prev_orientation = p_owner_rb->orientation;
			}*/
			p_owner_rb->orientation = p_attached_to_rb->orientation;
		}
		else {
			p_owner_transform->orientation = p_attached_to_transform->orientation;
		}
	}
}

void Attachment::SetAttachedTo(GameObject* p_attached_to) {
	// remove from old object holder
	if (p_go_attached_to) {
		ObjectHolder* p_old_obj_hold = p_go_attached_to->HasComponent<ObjectHolder>();
		p_old_obj_hold->RemoveAttachment(GetOwner());
	}

	// set new object holder
	p_go_attached_to = p_attached_to;
	ObjectHolder* p_new_obj_hold = p_go_attached_to->HasComponent<ObjectHolder>();
	p_new_obj_hold->AddAttachment(GetOwner());
}

GameObject* Attachment::GetAttachedTo() {
	return p_go_attached_to;
}

void Attachment::SetFollow(Bool const& _follow) {
	follow = _follow;
}

void Attachment::SetLockOrientation(Bool const& _lock_ori) {
	lock_orientation = _lock_ori;
}

void Attachment::SetOffset(Vec3 const& _offset) {
	pos_offset = _offset;
}

Vec3 const& Attachment::GetOffset() {
	return pos_offset;
}

BEGIN_ATTRIBUTES_FOR(Attachment)
DEFINE_MEMBER(Bool, follow)
DEFINE_MEMBER(Bool, lock_orientation)
DEFINE_MEMBER(Vec3, pos_offset)
END_ATTRIBUTES