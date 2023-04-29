#include "stdafx.h"

#include "PlayerCharacter.h"

#include "BaseState.h"
#include "Health.h"
#include "CarController.h"

#include "Engine/GameObject.h"
#include "Engine/ParticleSystem.h"
#include "Engine/ResourceManager.h"

PlayerCharacter::PlayerCharacter() :
	p_bullet_hit_emitter{ p_particle_system->NewEmitter() },
	p_smoke_emitter{ p_particle_system->NewEmitter() },
	p_flame_emitter{ p_particle_system->NewEmitter() }
{
	// bullet hit emitter settings
	p_bullet_hit_emitter->particles_per_sec = 10;
	p_bullet_hit_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_bullet_hit_emitter->color_gen_bnds = {
		.min = Vec4{ 0.96f, 0.05f, 0.07f, 1.0f },
		.max = Vec4{ 0.96f, 0.05f, 0.07f, 1.0f }
	};
	p_bullet_hit_emitter->speed_gen_bnds = {
		.min = 25.0f,
		.max = 30.0f
	};
	p_bullet_hit_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_bullet_hit_emitter->phi = {
		.min = glm::radians(-80.0f),
		.max = glm::radians(-50.0f)
	};
	p_bullet_hit_emitter->lifetime_secs_gen_bnds = {
		.min = 0.1f,
		.max = 0.3f
	};
	p_bullet_hit_emitter->uniform_scale_gen_bnds = {
		.min = 0.3f,
		.max = 0.3f
	};

	// smoke emitter
	p_smoke_emitter->particles_per_sec = 30;
	p_smoke_emitter->gravity_scale = 0.0f;
	p_smoke_emitter->texture = p_resource_manager->GetTexture("smoke_01.png");
	p_smoke_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_smoke_emitter->color_gen_bnds = {
		.min = Vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
		.max = Vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	p_smoke_emitter->speed_gen_bnds = {
		.min = 3.0f,
		.max = 4.0f
	};
	p_smoke_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_smoke_emitter->phi = {
		.min = glm::radians(-90.0f),
		.max = glm::radians(-45.0f)
	};
	p_smoke_emitter->lifetime_secs_gen_bnds = {
		.min = 1.0f,
		.max = 1.5f
	};
	p_smoke_emitter->uniform_scale_gen_bnds = {
		.min = 2.5f,
		.max = 2.75f
	};

	// flame emitter
	p_flame_emitter->particles_per_sec = 30;
	p_flame_emitter->gravity_scale = 0.0f;
	p_flame_emitter->texture = p_resource_manager->GetTexture("scorch_03.png");
	p_flame_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.25f, 0.0f }
	};
	p_flame_emitter->color_gen_bnds = {
		.min = Vec4{ 0.98f, 0.09f, 0.0f, 1.0f },
		.max = Vec4{ 0.98f, 0.39f, 0.0f, 1.0f }
	};
	p_flame_emitter->speed_gen_bnds = {
		.min = 2.0f,
		.max = 3.0f
	};
	p_flame_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_flame_emitter->phi = {
		.min = glm::radians(-90.0f),
		.max = glm::radians(-45.0f)
	};
	p_flame_emitter->lifetime_secs_gen_bnds = {
		.min = 0.15f,
		.max = 0.35f
	};
	p_flame_emitter->uniform_scale_gen_bnds = {
		.min = 1.5f,
		.max = 2.0f
	};
}

PlayerCharacter::~PlayerCharacter() noexcept {
	p_base_state->SetPlayerGameObjPtr(nullptr);
	p_particle_system->EraseEmitter(p_flame_emitter);
	p_particle_system->EraseEmitter(p_smoke_emitter);
	p_particle_system->EraseEmitter(p_bullet_hit_emitter);
}

void PlayerCharacter::Deserialize(rapidjson::Value const& json_value) {
	SIK_ASSERT(p_base_state->GetPlayerGameObjPtr() == nullptr, "Game already has a player game object assigned");
	//Sets the owner of this component as the player game objects
	p_base_state->SetPlayerGameObjPtr(GetOwner());
}

void PlayerCharacter::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
}

void PlayerCharacter::Modify(rapidjson::Value const& json_value) {
	SIK_ASSERT(p_base_state->GetPlayerGameObjPtr() == nullptr, "Game already has a player game object assigned");
	//Sets the owner of this component as the player game objects
	p_base_state->SetPlayerGameObjPtr(GetOwner());
}


void PlayerCharacter::Link() {
}

void PlayerCharacter::Enable() {
	// update meshes and emitters
	TakeDamage(0);
}

void PlayerCharacter::Disable() {
	p_smoke_emitter->is_active = false;
	p_flame_emitter->is_active = false;
}

void PlayerCharacter::Update(Float32 dt) {
	// update smoke emitter
	Transform* p_tr = GetOwner()->HasComponent<Transform>();
	p_smoke_emitter->position = p_tr->LocalToWorld(Vec3{ 0.0f, p_tr->scale.y, -p_tr->scale.z });
	p_smoke_emitter->orientation = p_tr->orientation;

	// update flame emitter
	p_flame_emitter->position = p_tr->LocalToWorld(Vec3{ 0.0f, p_tr->scale.y, -p_tr->scale.z });
	p_flame_emitter->orientation = p_tr->orientation;
}

void PlayerCharacter::TakeDamage(Int32 bullet_damage) {
	GameObject* owner = GetOwner();

	// health update
	Health* p_health = owner->HasComponent<Health>();
	p_health->TakeDamage(bullet_damage);
	SIK_INFO("Player Health: {}", p_health->GetCurrHP());

	MeshRenderer* mr = owner->HasComponent<MeshRenderer>();
	CarController* cc = owner->HasComponent<CarController>();

	// model change
	Int32 player_health = p_health->GetCurrHP();
	if (player_health <= 16) {
		mr->mesh = p_resource_manager->GetMesh("car_dmg_10.fbx");

		p_smoke_emitter->is_active = true;
		p_smoke_emitter->particles_per_sec = 50;
		p_flame_emitter->is_active = true;

		cc->SetHeadlights(false);
	}
	else if (player_health <= 33 &&
			 player_health > 16) {
		mr->mesh = p_resource_manager->GetMesh("car_dmg_33.fbx");

		p_smoke_emitter->is_active = true;
		p_smoke_emitter->particles_per_sec = 30;
		p_flame_emitter->is_active = false;

		cc->SetHeadlights(true);
	}
	else if (player_health <= 45 &&
			 player_health > 33) {
		mr->mesh = p_resource_manager->GetMesh("car_dmg_66.fbx");

		p_smoke_emitter->is_active = false;
		p_flame_emitter->is_active = false;

		cc->SetHeadlights(true);
	}
	else if (player_health > 45) {
		mr->mesh = p_resource_manager->GetMesh("car.fbx");

		p_smoke_emitter->is_active = false;
		p_flame_emitter->is_active = false;

		cc->SetHeadlights(true);
	}

	if (bullet_damage > 0) {
		// particle effect
		Transform* p_tr = GetOwner()->HasComponent<Transform>();
		p_bullet_hit_emitter->position = p_tr->position;
		p_bullet_hit_emitter->orientation = p_tr->orientation;
		p_bullet_hit_emitter->EmitParticles(15);
	}
}

BEGIN_ATTRIBUTES_FOR(PlayerCharacter)
END_ATTRIBUTES