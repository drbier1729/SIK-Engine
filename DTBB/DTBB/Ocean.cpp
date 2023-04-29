#include "stdafx.h"

#include "Ocean.h"

#include "Engine/GameObject.h"

Ocean::Ocean() :
	orig_pos{ 0.0f },
	orig_ori{},
	receding{false},
	raising{true},
	x_speed{ 0.0f },
	y_speed{ 0.0f }
{

}

void Ocean::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Ocean>(json_value, this);
}

void Ocean::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Ocean>(json_value, this, alloc);
}

void Ocean::Link() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	orig_pos = p_rb ? p_rb->position : p_tr->position;
	orig_ori = p_rb ? p_rb->orientation : p_tr->orientation;
}

void Ocean::Reset() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	if (p_rb) {
		p_rb->position = orig_pos;
		p_rb->orientation = orig_ori;
	}
	else {
		p_tr->position = orig_pos;
		p_tr->orientation = orig_ori;
	}
}

void Ocean::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void Ocean::Update(Float32 dt) {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();

	/*if (p_tr->position.x < (original_pos.x - 2.5f)) {
		receding = true;
	}
	if (p_tr->position.x > (original_pos.x + 2.5f)) {
		receding = false;
	}*/

	if (p_tr->position.y < (orig_pos.y - 2.5f)) {
		raising = true;
	}
	if (p_tr->position.y > (orig_pos.y + 2.5f)) {
		raising = false;
	}

	/*if (receding) {
		p_tr->SetPosition(p_tr->position.x + (x_speed * dt), p_tr->position.y, p_tr->position.z);
	}
	else {
		p_tr->SetPosition(p_tr->position.x - (x_speed * dt), p_tr->position.y, p_tr->position.z);
	}*/

	if (raising) {
		p_tr->SetPosition(p_tr->position.x, p_tr->position.y + (y_speed * dt), p_tr->position.z);
	}
	else {
		p_tr->SetPosition(p_tr->position.x, p_tr->position.y - (y_speed * dt), p_tr->position.z);
	}
}

BEGIN_ATTRIBUTES_FOR(Ocean)
DEFINE_MEMBER(Float32, x_speed)
DEFINE_MEMBER(Float32, y_speed)
END_ATTRIBUTES