#include "stdafx.h"

#include "CraneEnemy.h"

#include "Engine/GameObject.h"

CraneEnemy::CraneEnemy():
	orig_pos{ 0.0f },
	orig_ori{}
{
}

CraneEnemy::~CraneEnemy() noexcept
{
}

void CraneEnemy::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<CraneEnemy>(json_value, this);
}

void CraneEnemy::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<CraneEnemy>(json_value, this, alloc);
}

void CraneEnemy::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void CraneEnemy::OnCollide(GameObject* other) {
}

void CraneEnemy::Link() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	orig_pos = p_rb ? p_rb->position : p_tr->position;
	orig_ori = p_rb ? p_rb->orientation : p_tr->orientation;
}

void CraneEnemy::Enable() {
	
}

void CraneEnemy::Disable()
{
}

void CraneEnemy::Reset() {
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

void CraneEnemy::Update(Float32 dt) {
}

void CraneEnemy::FixedUpdate(Float32 dt) {
}

BEGIN_ATTRIBUTES_FOR(CraneEnemy)
END_ATTRIBUTES