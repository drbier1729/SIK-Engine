#include "stdafx.h"

#include "TurretEnemy.h"

#include <glm/gtx/vector_angle.hpp>

#include "Engine/GameObjectManager.h"
#include "Engine/ParticleSystem.h"
#include "Engine/MotionProperties.h"
#include "Engine/AudioManager.h"
#include "Engine/InputManager.h"
#include "Engine/Factory.h"
#include "Engine/PhysicsManager.h"
#include "Engine/ResourceManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/MeshRenderer.h"

#include "Health.h"
#include "BallnChain.h"
#include "PlayerCharacter.h"
#include "BaseState.h"
#include "GamePlayState.h"
#include "Collectable.h"
#include "Destroyable.h"
#include "ObjectHolder.h"
#include "Debris.h"

TurretEnemy::TurretEnemy() :
	p_target{ nullptr },
	p_turret_emitter{ p_particle_system->NewEmitter() },
	p_hit_emitter{ p_particle_system->NewEmitter() },
	p_death_emitter{ p_particle_system->NewEmitter() },
	p_spark_emitter{ p_particle_system->NewEmitter() },
	p_smoke_emitter{ p_particle_system->NewEmitter() },
	p_flame_emitter{ p_particle_system->NewEmitter() },
	is_alive{ true },
	is_aggro{ false },
	collided_last_frame{ false },
	collided_this_frame{ false },
	dead_particles_timer{ 0.0f },
	spark_particles_timer{ 0.0f },
	invulnerability_timer{ 1.0f },
	death_timer{ 0.0f },
	p_light{ p_graphics_manager->AddLocalLight() },
	aggro_radius{ 0.0f },
	bullet_damage{ 0 },
	damage_momentum{ 500.0f },
	secs_per_bullet{ 0.0f },
	light_col{ 0.0f },
	explosion_radius{ 15.0f }
{
	SetupParticleEmitters();
}

TurretEnemy::~TurretEnemy() noexcept {
	p_particle_system->EraseEmitter(p_flame_emitter);
	p_particle_system->EraseEmitter(p_smoke_emitter);
	p_particle_system->EraseEmitter(p_spark_emitter);
	p_particle_system->EraseEmitter(p_death_emitter);
	p_particle_system->EraseEmitter(p_hit_emitter);
	p_particle_system->EraseEmitter(p_turret_emitter);
	p_graphics_manager->RemoveLocalLight(p_light);
}

void TurretEnemy::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<TurretEnemy>(json_value, this);

	target_name = json_value.FindMember("target_name")->value.GetString();
}

void TurretEnemy::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<TurretEnemy>(json_value, this, alloc);
}

void TurretEnemy::Link() {
	// get target game object ptr
	auto& go_pool = p_game_obj_manager->GetGameObjectContainer();
	for (auto r = go_pool.all(); not r.is_empty(); r.pop_front()) {
		GameObject& go_ref = r.front();
		if (go_ref.GetName().compare(target_name) == 0) {
			p_target = &go_ref;
		}
	}

	// scripts
	Behaviour* p_behaviour = GetOwner()->HasComponent<Behaviour>();

	p_behaviour->SetStateVariable(aggro_radius * aggro_radius, "radius_sq");
	p_behaviour->SetStateVariable(FLT_MAX, "dist_sq");
	for (auto& script : p_behaviour->scripts) {
		script.script_state.set_function("SetAggro", &TurretEnemy::SetAggro, this);
	}
	
	// set firing rate
	p_turret_emitter->particles_per_sec = 1.0f / secs_per_bullet;

	// Light Settings
	p_light->color = light_col;
	p_light->radius = aggro_radius;
	p_light->ease_out = false;
	// cannot set light position here as transform has
	// not been updated by the Attachment comp yet
}

// called by ObjectHolder (turret base) OnCollide
void TurretEnemy::OnCollide(GameObject* other) {
	if (is_alive) {
		// if other object is a wrecking ball
		if (other->HasComponent<BallnChain>()) {
			RigidBody* other_rb = other->HasComponent<RigidBody>();
			Float32 curr_momentum = glm::length((other_rb->motion_props)->linear_velocity) * other_rb->motion_props->mass;
			// wrecking ball momentum is above threshold
			if (curr_momentum > damage_momentum) {
				collided_this_frame = true;
			}
		}

		// if other object is a debris object
		if (Debris* p_debris = other->HasComponent<Debris>()) {
			RigidBody* other_rb = other->HasComponent<RigidBody>();
			Float32 other_vel = glm::length(other_rb->motion_props->linear_velocity);
			// debris speed is more than required
			if (other_vel > 4.0f && (not p_debris->IsStuck())) {
				collided_this_frame = true;
			}
		}
	}
}
void TurretEnemy::Update(Float32 dt) {
	GameObject* owner = GetOwner();
	Behaviour* p_behaviour = owner->HasComponent<Behaviour>();
	Transform* p_tr = owner->HasComponent<Transform>();

	if ( IsDying() ) {
		death_timer -= dt;
		if (death_timer <= 0.0f) {
			SayGoodbyeAndDie();
		}
	}

	// Light position needs to be set here because
	// the Attachment comp only updates the transform in Update()
	p_light->position = p_tr->position;
	p_light->position.y += 1.0f;
	//p_light->position.y = -1.0f;

	if (is_alive) {
		if (collided_last_frame == false && collided_this_frame == true) {// collision enter
			if (invulnerability_timer <= 0.01f) {
				invulnerability_timer = 1.0f;

				Health* p_health = owner->HasComponent<Health>();
				p_health->TakeDamage(1);

				SIK_INFO("Enemy Health: {}", p_health->GetCurrHP());
				if (p_health->GetCurrHP() <= 0) {
					SayGoodbyeAndDie();
				}
				else {
					// Play destroyed sound effect
					p_audio_manager->PlayAudio("TURRET_DES"_sid,
						p_audio_manager->sfx_chanel_group,
						p_audio_manager->turret_destroy_vol,
						0.8f,
						false,
						0);

					// Rumbling lit bit for a hit
					p_input_manager->RumbleController(100, 1000, 225);

					// Reduce firing rate
					secs_per_bullet *= 2.0f;
					p_turret_emitter->particles_per_sec = 1.0f / secs_per_bullet;

					// enable smoke
					p_smoke_emitter->position = p_tr->LocalToWorld(Vec3{ 0.0f, p_tr->scale.y, 0.0f });
					p_smoke_emitter->orientation = p_tr->orientation;
					p_smoke_emitter->is_active = true;

					// damaged model
					MeshRenderer* mr = owner->HasComponent<MeshRenderer>();
					mr->mesh = p_resource_manager->GetMesh("turret_top_dmg_50.fbx");
				}

				// hit emitter
				p_hit_emitter->position = p_tr->position;
				p_hit_emitter->orientation = p_tr->orientation;
				p_hit_emitter->EmitParticles(15);
			}
		}

		if (collided_last_frame == true && collided_this_frame == true) {// collision stay
			
		}

		if (collided_last_frame == true && collided_this_frame == false) {// collision exit
			
		}

		if (collided_last_frame == false && collided_this_frame == false) {// no collision			
			// Get target info
			Transform* p_target_tr = p_target->HasComponent<Transform>();
			Vec3 target_pos = p_target_tr->position;

			Vec3 turret_to_target = target_pos - p_tr->position;

			p_turret_emitter->is_active = is_aggro;
			if (is_aggro) {
				// turn light red
				p_light->color = Vec3{ light_col.r * 1.5f, light_col.g, light_col.b };

				// shoot at target
				Vec3 final_dir = turret_to_target;
				final_dir.y = 0.0f;
				final_dir = glm::normalize(final_dir);
				Vec3 curr_dir = glm::normalize(p_tr->orientation * Vec3{ 0.0f, 0.0f, -1.0f });

				Float32 y_angle = glm::angle(final_dir, glm::normalize(turret_to_target));

				p_turret_emitter->phi = {
					.min = y_angle - glm::radians(1.0f),
					.max = y_angle + glm::radians(1.0f)
				};

				// calculate x_angle
				Vec2 final_vec2 = Vec2{ final_dir.x, final_dir.z };
				Vec2 curr_vec2 = Vec2{ curr_dir.x, curr_dir.z };
				Float32 dot = glm::dot(final_vec2, curr_vec2);
				Float32 det = final_vec2.x * curr_vec2.y - final_vec2.y * curr_vec2.x;
				Float32 x_angle = atan2f(det, dot);

				p_tr->orientation = glm::rotate(p_tr->orientation, x_angle * dt * 2.0f, Vec3{ 0.0f, 1.0f, 0.0f });

				p_turret_emitter->position = p_tr->position + (curr_dir * p_tr->scale.z * 2.0f);
				p_turret_emitter->orientation = p_tr->orientation;

				// spark emitters
				if (spark_particles_timer > secs_per_bullet) {
					// Nail Shooting sound effect
					p_audio_manager->PlayAudio("NAIL_SHOOT"_sid,
						p_audio_manager->sfx_chanel_group,
						p_audio_manager->turret_shoot_vol,
						1.0f,
						false,
						0);

					p_spark_emitter->position = p_turret_emitter->position;
					p_spark_emitter->orientation = p_tr->orientation;
					p_spark_emitter->EmitParticles(10);

					spark_particles_timer = 0.0f;
				}
				spark_particles_timer += dt;
			}
			else {
				p_light->color = light_col;
			}

			// aggro check
			Float32 dist_sq = glm::length2(turret_to_target);
			p_behaviour->SetStateVariable(dist_sq, "dist_sq");

			/*
			* script compares dist_sq and radius_sq and set aggro state
			*/
		}

		collided_last_frame = collided_this_frame;
		collided_this_frame = false;

		// reduce invulnerabilty timer
		invulnerability_timer -= dt;
		// ensure it does not overflow
		if (invulnerability_timer < -1.0f) {
			invulnerability_timer = 0.0f;
		}
	}
	else {
		// "dead" behaviour
		// turret emitter
		p_turret_emitter->is_active = false;

		// once every 0.8 secs
		if (dead_particles_timer > 0.8f) {
			// death emitter
			p_spark_emitter->position = p_tr->position;
			p_spark_emitter->orientation = p_tr->orientation;
			p_spark_emitter->EmitParticles(15);

			dead_particles_timer = 0.0f;
		}

		dead_particles_timer += dt;
	}
}

void TurretEnemy::SetAggro(Bool _is_aggro) {
	is_aggro = _is_aggro;
}

Bool TurretEnemy::IsAlive() const {
	return is_alive;
}

Bool TurretEnemy::IsDying() const {
	return death_timer > 0.0f;
}

void TurretEnemy::SetDeathTimer(Float32 when_secs) {
	death_timer = when_secs;
}

void TurretEnemy::Disable() {
	p_light->enabled = false;

	// disable emitters
	p_smoke_emitter->is_active = false;
	p_flame_emitter->is_active = false;
	p_turret_emitter->is_active = false;
}

void TurretEnemy::Enable() {
	p_light->enabled = true;

	GameObject* p_owner = GetOwner();
	Health* p_health = p_owner->HasComponent<Health>();
	
	if (p_health->GetCurrHP() < 2) {
		p_smoke_emitter->is_active = true;
	}

	if (p_health->GetCurrHP() < 1) {
		p_flame_emitter->is_active = true;
		p_light->enabled = false;
	}
}

void TurretEnemy::Reset() {
	is_alive = true;
	//Restore the turret health
	GameObject* p_owner = GetOwner();

	//Reset the turret model
	// change model
	MeshRenderer* mr = p_owner->HasComponent<MeshRenderer>();
	mr->mesh = p_resource_manager->GetMesh("turret_top.fbx");

	// disable emitters
	p_smoke_emitter->is_active = false;
	p_flame_emitter->is_active = false;
	p_turret_emitter->is_active = false;

	p_light->enabled = true;
}

void TurretEnemy::SayGoodbyeAndDie() {

	if (!is_alive) {
		return;
	}

	is_alive = false;

	// Play destroyed sound effect
	p_audio_manager->PlayAudio("TURRET_DES"_sid,
		p_audio_manager->sfx_chanel_group,
		p_audio_manager->turret_destroy_vol,
		1.5f,
		false,
		0);

	p_base_state->GetGamePlayState()->DecrementTurretCount();

	//Rumble longer if turret dies
	p_input_manager->RumbleController(500, 5000, 1000);

	// Fetch owner data
	GameObject* owner = GetOwner();
	if (not owner) { return; }
	Transform* tr = owner->HasComponent<Transform>();

	// change model
	MeshRenderer* mr = owner->HasComponent<MeshRenderer>();
	mr->mesh = p_resource_manager->GetMesh("turret_top_dmg_0.fbx");

	// disable light
	p_light->enabled = false;

	// enable smoke
	p_smoke_emitter->position = tr->LocalToWorld(Vec3{ 0.0f, tr->scale.y, 0.0f });
	p_smoke_emitter->orientation = tr->orientation;
	p_smoke_emitter->is_active = true;

	// enable flame
	p_flame_emitter->position = tr->LocalToWorld(Vec3{ 0.0f, tr->scale.y, 0.0f });
	p_flame_emitter->orientation = tr->orientation;
	p_flame_emitter->is_active = true;

	// Emit explosion particles
	p_death_emitter->position = tr->position;
	p_death_emitter->orientation = tr->orientation;
	p_death_emitter->EmitParticles(512);

	// Spawn Health Collectible object here
	GameObject* p_col_obj = p_factory->BuildGameObject("HealthCollectible.json");
	if (p_col_obj) {
		RigidBody* col_rb = p_col_obj->HasComponent<RigidBody>();
		Collectable* col = p_col_obj->HasComponent<Collectable>();

		if (col_rb && col) {
			col_rb->position = tr->position;
			col_rb->Enable(true);

			col->SetStartHeight(col_rb->position.y);
			col->SetTrailColor(Vec3{ 0.0f, 0.95f, 0.2f });
		}

		p_base_state->GetGamePlayState()->AddObject(p_col_obj);
	}

	// Destroy neighboring turrets and destroyables
	p_physics_manager->ForEachInRadius(explosion_radius, tr->position, 
		[owner](RigidBody const& rb) -> Bool {
			if (not rb.owner || rb.owner == owner) { return true; }

			if (TurretEnemy* other_turret = rb.owner->HasComponent<TurretEnemy>(); other_turret) {
				if (other_turret->IsAlive() && not other_turret->IsDying()) { // check to avoid infinite loops!
					other_turret->SetDeathTimer(0.5f);
				}
			}
			
			if (Destroyable* other_destroyable = rb.owner->HasComponent<Destroyable>(); other_destroyable) {
				other_destroyable->SayGoodbyeAndDie();
			}

			// TODO: remove this if possible. This is needed for now because Turrets 
			// are weird... Adding a RigidBody to them in the JSON resulted in strange 
			// behavior in the scene (things spawned in incorrect locations, collisions
			// failed to happen, etc.)
			ObjectHolder* other_holder = rb.owner->HasComponent<ObjectHolder>();
			if (other_holder) {
				auto const& attached = other_holder->GetAttachedObjects();
				for (auto&& go : attached) {
					if (not go) { continue; }

					if (TurretEnemy* t = go->HasComponent<TurretEnemy>(); t) {
						if (t->IsAlive() && not t->IsDying()) { // check to avoid infinite loops!
							t->SetDeathTimer(0.5f);
						}
					}
					if (Destroyable* d = go->HasComponent<Destroyable>(); d) {
						d->SayGoodbyeAndDie();
					}
				}
			}

			return true;
		}
	);
}

void TurretEnemy::SetupParticleEmitters() {
	// turret emitter settings
	p_turret_emitter->particles_per_sec = 1.0f / secs_per_bullet;
	p_turret_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_turret_emitter->color_gen_bnds = {
		.min = Vec4{ 0.9f, 0.3f, 0.3f, 1.0f },
		.max = Vec4{ 0.9f, 0.3f, 0.3f, 1.0f }
	};
	p_turret_emitter->speed_gen_bnds = {
		.min = 30.0f,
		.max = 30.0f
	};
	p_turret_emitter->theta = {
		.min = glm::radians(-1.0f),
		.max = glm::radians(1.0f)
	};
	p_turret_emitter->lifetime_secs_gen_bnds = {
		.min = 2.0f,
		.max = 2.0f
	};
	p_turret_emitter->uniform_scale_gen_bnds = {
		.min = 0.4f,
		.max = 0.4f
	};
	p_turret_emitter->UpdateParticle = [this](Particle& p) {
		// disable if below ground
		if (p.position.y < -1.0f) {
			p.secs_remaining = 0.0f;
			return;
		}

		// check for collision with player
		Collision::AABB tmp{};
		tmp.halfwidths = Vec3{ p.uniform_scale };
		tmp.position = p.position;

		RigidBody* p_target_rb = p_target->HasComponent<RigidBody>();
		if (tmp.Intersects(p_target_rb->bounds)) {
			// player takes damage
			PlayerCharacter* p_player = p_target->HasComponent<PlayerCharacter>();
			p_player->TakeDamage(bullet_damage);
			p.secs_remaining = 0.0f;
		}
	};

	// hit emitter settings
	p_hit_emitter->particles_per_sec = 20;
	p_hit_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_hit_emitter->color_gen_bnds = {
		.min = Vec4{ 0.97f, 0.1f, 0.12f, 1.0f },
		.max = Vec4{ 0.97f, 0.1f, 0.12f, 1.0f }
	};
	p_hit_emitter->speed_gen_bnds = {
		.min = 20.0f,
		.max = 25.0f
	};
	p_hit_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_hit_emitter->phi = {
		.min = glm::radians(-70.0f),
		.max = glm::radians(-30.0f)
	};
	p_hit_emitter->lifetime_secs_gen_bnds = {
		.min = 0.5f,
		.max = 1.25f
	};
	p_hit_emitter->uniform_scale_gen_bnds = {
		.min = 0.3f,
		.max = 0.4f
	};

	// death emitter settings
	p_death_emitter->particles_per_sec = 15;
	p_death_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_death_emitter->color_gen_bnds = {
		.min = Vec4{ 0.9f, 0.0, 0.0, 1.0 },
		.max = Vec4{ 0.9f, 0.1, 0.0, 1.0 }
	};
	p_death_emitter->speed_gen_bnds = {
		.min = 15.0f,
		.max = 15.0f
	};
	p_death_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_death_emitter->phi = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(-60.0f)
	};
	p_death_emitter->lifetime_secs_gen_bnds = {
		.min = 1.3f,
		.max = 1.5f
	};
	p_death_emitter->uniform_scale_gen_bnds = {
		.min = 2.0f,
		.max = 4.0f
	};
	p_death_emitter->texture = p_resource_manager->GetTexture("flame_particle.png"_sid);

	// spark emitter settings
	p_spark_emitter->particles_per_sec = 10;
	p_spark_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_spark_emitter->color_gen_bnds = {
		.min = Vec4{ 0.1f, 0.01f, 0.01f, 1.0f },
		.max = Vec4{ 0.1f, 0.01f, 0.01f, 1.0f }
	};
	p_spark_emitter->speed_gen_bnds = {
		.min = 10.0f,
		.max = 10.0f
	};
	p_spark_emitter->lifetime_secs_gen_bnds = {
		.min = 0.3f,
		.max = 0.3f
	};
	p_spark_emitter->uniform_scale_gen_bnds = {
		.min = 0.3f,
		.max = 0.3f
	};

	// smoke emitter settings
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
		.max = glm::radians(-20.0f)
	};
	p_smoke_emitter->lifetime_secs_gen_bnds = {
		.min = 1.0f,
		.max = 1.5f
	};
	p_smoke_emitter->uniform_scale_gen_bnds = {
		.min = 1.5f,
		.max = 2.5f
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

BEGIN_ATTRIBUTES_FOR(TurretEnemy)
DEFINE_MEMBER(Float32, aggro_radius)
DEFINE_MEMBER(Int32, bullet_damage)
DEFINE_MEMBER(Float32, damage_momentum)
DEFINE_MEMBER(Float32, secs_per_bullet)
DEFINE_MEMBER(Vec3, light_col)
DEFINE_MEMBER(Float32, explosion_radius)
END_ATTRIBUTES