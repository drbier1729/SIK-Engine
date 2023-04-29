#include "stdafx.h"

#include "Turret.h"

#include "Engine/ParticleSystem.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObject.h"
#include "Engine/GameObjectManager.h"
#include "Engine/RigidBody.h"
#include "Engine/MemoryManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/InputAction.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"

Turret::Turret():
	p_emitter{ p_particle_system->NewEmitter() },
	turret_actions{ p_memory_manager->GetCurrentAllocator().new_object<InputAction>("CombatTurretControls.json") },
	kill_count{ 0 }
{
	// emitter settings
	p_emitter->particles_per_sec = 15;
	p_emitter->texture = p_resource_manager->GetTexture("DigiPen_RGB_White.png");
	/*p_emitter->vel_gen_bnds = {
		.min = Vec3{ 0.0f, 0.2f, 0.0f },
		.max = Vec3{ 0.0f, 0.2f, 0.0f }
	};*/
	p_emitter->pos_gen_bnds = {
		.min = Vec3{ 0 },
		.max = Vec3{ 0 }
	};
	p_emitter->color_gen_bnds = {
		.min = Vec4{ 1 },
		.max = Vec4{ 1 }
	};
	p_emitter->lifetime_secs_gen_bnds = {
		.min = 2.0f,
		.max = 2.0f
	};
	p_emitter->uniform_scale_gen_bnds = {
		.min = 0.3f,
		.max = 0.3f
	};
	p_emitter->UpdateParticle = [this](Particle& p) {
		Collision::AABB tmp{};
		tmp.halfwidths = Vec3{ p_emitter->uniform_scale_gen_bnds.max };
		tmp.position = p.position;

		// for every enemy present
		for (auto&& e : enemies) {
			if (e != nullptr) {
				RigidBody* p_enemy_rb = e->HasComponent<RigidBody>();
				if (p_enemy_rb != nullptr) {
					// check if particle collides with enemy
					if (tmp.Intersect(p_enemy_rb->bounds)) {
						e->Disable();
						++kill_count;
						p.secs_remaining = 0.0f;
						RemoveEnemy(e);
						break;
					}
				}
			}
		}
	};
}

Turret::~Turret() noexcept {
	p_particle_system->EraseEmitter(p_emitter);
	p_memory_manager->GetCurrentAllocator().delete_object(turret_actions);
}

void Turret::Update(Float32 dt) {
	GameObject* p_owner = GetOwner();

	Transform* p_transform = p_owner->HasComponent<Transform>();

	p_emitter->position = p_transform->position;
	Vec3 direction = p_transform->orientation * Vec3{ 0.0f, 0.0f, -1.0f };
	direction = glm::normalize(direction);
	//p_emitter->vel_gen_bnds.min.x = 30.0f * direction.x;
	//p_emitter->vel_gen_bnds.min.z = 30.0f * direction.z;
	//p_emitter->vel_gen_bnds.max.x = 30.0f * direction.x;
	//p_emitter->vel_gen_bnds.max.z = 30.0f * direction.z;

	// update GUI with kill count
	p_gui_object_manager->GetTextObject(1)->SetText(std::to_string(kill_count).c_str());
}

InputAction* Turret::GetInputAction() const {
	return turret_actions;
}

void Turret::SetFiring(Bool is_firing) {
	p_emitter->is_active = is_firing;
}

void Turret::AddEnemy(GameObject* enemy) {
	enemies.push_back(enemy);
}

void Turret::RemoveEnemy(GameObject* enemy) {
	auto b_enem = enemies.begin();
	auto e_enem = enemies.end();
	for (; b_enem != e_enem; ++b_enem) {
		if (*b_enem == enemy) {
			enemies.erase(b_enem);
			break;
		}
	}
}

Bool Turret::HasEnemies() const {
	return (enemies.size() > 0);
}

BEGIN_ATTRIBUTES_FOR(Turret)
END_ATTRIBUTES