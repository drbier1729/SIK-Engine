#include "stdafx.h"

#include "Enemy.h"

#include "Engine/GameObject.h"
#include "Player.h"
#include "Turret.h"

Enemy::Enemy():
	player_obj{ nullptr },
	p_turret{ nullptr }
{

}

void Enemy::Deserialize(rapidjson::Value const& json_value) {

}

void Enemy::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {

}

void Enemy::OnCollide(GameObject* other) {
	if (other == player_obj) {
		GameObject* owner = GetOwner();
		p_turret->RemoveEnemy(owner);
		owner->Disable();
	}
}

void Enemy::Update(Float32 dt) {
	GameObject* owner = GetOwner();
	RigidBody* p_rigidbody = owner->HasComponent<RigidBody>();

	Transform* p_player_tr = player_obj->HasComponent<Transform>();

	// move dir = target position - current position
	Vec3 move_dir = p_player_tr->position - p_rigidbody->position;
	move_dir = glm::normalize(move_dir);

	// move
	p_rigidbody->position += move_dir * dt;
}

BEGIN_ATTRIBUTES_FOR(Enemy)
END_ATTRIBUTES