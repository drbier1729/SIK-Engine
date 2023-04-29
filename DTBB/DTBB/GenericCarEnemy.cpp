#include "stdafx.h"

#include "GenericCarEnemy.h"

#include "Engine/GameObject.h"
#include "Engine/MotionProperties.h"
#include "Engine/ParticleSystem.h"
#include "Engine/ResourceManager.h"
#include "Engine/MeshRenderer.h"
#include "Engine/AudioManager.h"
#include "Engine/GraphicsManager.h"

#include "BaseState.h"
#include "GamePlayState.h"
#include "PlayerCharacter.h"
#include "BallnChain.h"
#include "Health.h"
#include "Debris.h"

GenericCarEnemy::GenericCarEnemy():
	enemy_type{ EnemyType::SmallCar },
	target_go{ nullptr },
	target_tr{ nullptr },
	local_forward{ 0 },
	local_right{ 0 },
	forward_force{ 0.0f },
	right_force{ 0.0f },
	move_angle{ 0.0f },
	is_alive{ true },
	is_chasing{ true },
	is_aggro{ false },
	collided_last_frame{ false },
	collided_this_frame{ false },
	player_collision{ false },
	self_damaging_collision{ false },
	orig_pos{ 0.0f },
	orig_ori{},
	rand_float{ std::make_unique<UniformRandFloat32>() },
	p_smoke_emitter{ p_particle_system->NewEmitter() },
	p_flame_emitter{ p_particle_system->NewEmitter() },
	p_headlights{ p_graphics_manager->AddConeLocalLight(), p_graphics_manager->AddConeLocalLight() },
	headlights_forward_offset{ 3.0f },
	headlights_vertical_offset{ 0.0f },
	headlights_width{ 1.5f },
	headlights_radius{ 0.5f },
	headlights_length{ 1.0f },
	in_same_spot{ false },
	in_same_spot_timer{ 3.0f },
	prev_pos{ 0.0f },
	reverse_timer{ 2.0f },
	turn_direction{ 1 },
	max_forward_speed{ 0.0f },
	max_reverse_speed{ 0.0f },
	max_angular_speed{ 0.0f },
	aggro_radius{ 0.0f },
	damage{ 0 },
	destroy_vel_threshold{ 0.0f }
{
	SetupEmitters();
}

GenericCarEnemy::~GenericCarEnemy() noexcept {
	p_particle_system->EraseEmitter(p_flame_emitter);
	p_particle_system->EraseEmitter(p_smoke_emitter);

	p_graphics_manager->RemoveConeLocalLight(p_headlights[0]);
	p_graphics_manager->RemoveConeLocalLight(p_headlights[1]);
}

void GenericCarEnemy::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<GenericCarEnemy>(json_value, this);

	target_go = p_base_state->GetPlayerGameObjPtr();
	target_tr = target_go->HasComponent<Transform>();

	// enemy type
	auto enemy_type_iter = enemy_types_map.find(
		ToStringID(json_value.FindMember("enemy_type")->value.GetString()));

	SIK_ASSERT(enemy_type_iter != enemy_types_map.end(), "Failed to find enemy type");

	enemy_type = enemy_type_iter->second;

	p_headlights[0]->radius = headlights_radius;
	p_headlights[1]->radius = headlights_radius;
	p_headlights[0]->length = headlights_length;
	p_headlights[1]->length = headlights_length;
	p_headlights[0]->color = headlights_color;
	p_headlights[1]->color = headlights_color;

}

void GenericCarEnemy::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<GenericCarEnemy>(json_value, this, alloc);
}

void GenericCarEnemy::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

void GenericCarEnemy::OnCollide(GameObject* other) {
	if (is_alive) {
		if (is_chasing) {
			// if other object is the target
			if (other == target_go) {
				collided_this_frame = true;
				player_collision = true;
			}
		}

		BallnChain* p_ball_n_chain = other->HasComponent<BallnChain>();
		Debris* p_debris = other->HasComponent<Debris>();

		// if other if a wrecking ball or is a thrown debris
		if (p_ball_n_chain || p_debris) {
			// other values
			RigidBody* other_rb = other->HasComponent<RigidBody>();
			if (!(other_rb && other_rb->motion_props)) { return; }

			Vec3 other_vel = other_rb->motion_props->linear_velocity;
			Float32 other_speed = glm::length(other_vel);

			// other body is not static and there is enough speed difference
			if (other_speed > destroy_vel_threshold) {
				// if debris, it should not be stuck to magnet (it must be thrown)
				if (p_debris) {
					if (not p_debris->IsStuck()) {
						collided_this_frame = true;
						self_damaging_collision = true;
					}
				}
				// if wrecking ball, no further checks
				else {
					collided_this_frame = true;
					self_damaging_collision = true;
				}
			}
		}
	}
}

void GenericCarEnemy::Link() {
	// scripts
	Behaviour* p_behaviour = GetOwner()->HasComponent<Behaviour>();
	SIK_ASSERT(p_behaviour != nullptr, "Must have a behaviour.");

	p_behaviour->SetStateVariable(aggro_radius * aggro_radius, "radius_sq");
	p_behaviour->SetStateVariable(FLT_MAX, "dist_sq");
	for (auto& script : p_behaviour->scripts) {
		script.script_state.set_function("SetAggro", &GenericCarEnemy::SetAggro, this);
	}

	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();
	orig_pos = p_rb->position;
	orig_ori = p_rb->orientation;
}

void GenericCarEnemy::Enable() {
	// updating health so we can determine state of emitters
	HealthUpdate(0);
	SetHeadlights(true);

	// updating state of emitters
	Health* p_health = GetOwner()->HasComponent<Health>();
	Int32 hp_curr = p_health->GetCurrHP();
	Int32 hp_max = p_health->GetMaxHP();
	Float32 health_ratio = static_cast<Float32>(hp_curr) / static_cast<Float32>(hp_max);

	switch (enemy_type) {
	case EnemyType::SmallCar: {
		if (health_ratio < 1.0f &&
			health_ratio >= 0.5f) {
			p_smoke_emitter->is_active = true;
			p_flame_emitter->is_active = false;
		}
		else if (health_ratio < 0.5f &&
			health_ratio >= 0.0f) {
			p_smoke_emitter->is_active = true;
			p_flame_emitter->is_active = true;
			SetHeadlights(false);
		}
		break;
	}
	case EnemyType::BigCar: {
		if (health_ratio < 0.66f &&
			health_ratio >= 0.33f) {
			p_smoke_emitter->is_active = true;
			p_flame_emitter->is_active = false;
		}
		else if (health_ratio < 0.33f &&
			health_ratio >= 0.0f) {
			p_smoke_emitter->is_active = true;
			p_flame_emitter->is_active = true;
			SetHeadlights(false);
		}
		break;
	}
	default: {
		break;
	}
	}
}

void GenericCarEnemy::Disable() {
	p_smoke_emitter->is_active = false;
	p_flame_emitter->is_active = false;

	SetHeadlights(false);
}

void GenericCarEnemy::Reset() {
	GameObject* p_owner = GetOwner();

	RigidBody* p_rb = p_owner->HasComponent<RigidBody>();

	p_rb->position = orig_pos;
	p_rb->orientation = orig_ori;

	MotionProperties* mp = p_rb->motion_props;
	if (mp) {
		mp->prev_position = p_rb->position;
		mp->linear_velocity = Vec3{ 0.0f };
		mp->angular_velocity = Vec3{ 0.0f };
	}

	is_alive = true;

	MeshRenderer* p_mr = p_owner->HasComponent<MeshRenderer>();
	switch (enemy_type) {
	break; case EnemyType::SmallCar: {
		p_mr->mesh = p_resource_manager->GetMesh("muscle_car.fbx");
	}
	break; case EnemyType::BigCar: {
		p_mr->mesh = p_resource_manager->GetMesh("monster_truck.fbx");
	}
	}

	p_smoke_emitter->is_active = false;
	p_flame_emitter->is_active = false;
}

void GenericCarEnemy::Update(Float32 dt) {
	GameObject* owner = GetOwner();
	Transform* p_transform = owner->HasComponent<Transform>();
	Behaviour* p_behaviour = owner->HasComponent<Behaviour>();

	if (is_alive) {
		if (is_chasing) {
			// get target position
			Vec3 target_pos = target_tr->position;
			Vec3 final_dir = target_pos - p_transform->position;

			// aggro check
			Float32 dist_sq = glm::length2(final_dir);
			p_behaviour->SetStateVariable(dist_sq, "dist_sq");

			if (is_aggro) {
				ChaseTarget(final_dir, dt);
			}

			// collision enter
			// stop chasing, set random location
			if (collided_last_frame == false &&
				collided_this_frame == true) {
				is_chasing = false;

				if (player_collision) {
					// damage player
					PlayerCharacter* pc = target_go->HasComponent<PlayerCharacter>();
					pc->TakeDamage(damage);

					player_collision = false;
				}
				else if (self_damaging_collision) {
					HealthUpdate(1);

					self_damaging_collision = false;
				}
			}
		}
		else {
			// go to random location
			Vec3 final_dir = orig_pos - p_transform->position;
			ChaseTarget(final_dir, dt);

			// reached, can chase again
			if (CloseEnough(p_transform->position, orig_pos)) {
				is_chasing = true;
			}
		}

		collided_last_frame = collided_this_frame;
		collided_this_frame = false;
	}

	// update emitters
	Vec3 hood_pos{ p_transform->LocalToWorld(Vec3{ 0.0f, p_transform->scale.y, -p_transform->scale.z }) };
	p_smoke_emitter->position = hood_pos;
	p_smoke_emitter->orientation = p_transform->orientation;

	p_flame_emitter->position = hood_pos;
	p_flame_emitter->orientation = p_transform->orientation;
	
	UpdateHeadlightPositions();
}

void GenericCarEnemy::FixedUpdate(Float32 dt) {
}

std::unordered_map<StringID, GenericCarEnemy::EnemyType> GenericCarEnemy::enemy_types_map {
	{"SmallCar"_sid, GenericCarEnemy::EnemyType::SmallCar},
	{"BigCar"_sid, GenericCarEnemy::EnemyType::BigCar}
};

GenericCarEnemy::EnemyType GenericCarEnemy::GetEnemyType() {
	return enemy_type;
}

void GenericCarEnemy::SetAggro(Bool in_range) {
	if (is_chasing) {
		is_aggro = in_range;
	}
	else {
		is_aggro = false;
	}
}

void GenericCarEnemy::ChaseTarget(Vec3 const& final_dir, Float32 dt) {
	GameObject* owner = GetOwner();
	RigidBody* p_rigidbody = owner->HasComponent<RigidBody>();
	MotionProperties* p_motion_props = p_rigidbody->motion_props;
	if (!(owner && p_rigidbody && p_motion_props)) { return; }

	// local vectors
	local_forward = glm::rotate(Mat4{ 1.0f }, move_angle, Vec3{ 0.0f, 1.0f, 0.0f })
		* Vec4 {
		0.0f, 0.0f, -1.0f, 0.0f
	};
	local_right = Vec3{ -local_forward.z, 0.0f, local_forward.x };

	// Velocity in local ref frame
	Float32 vel_right = glm::dot(p_motion_props->linear_velocity, local_right);
	Float32 vel_forward = glm::dot(p_motion_props->linear_velocity, local_forward);
	Float32 speed = std::sqrtf(vel_forward * vel_forward + vel_right * vel_right);

	// Resistance (drag, rolling resistance, friction) in local ref frame
	Float32 resistance_long = p_motion_props->linear_damping * vel_forward * std::fabsf(vel_forward) +
		(30.0f * p_motion_props->linear_damping) * vel_forward;
	Float32 resistance_lat = p_motion_props->linear_damping * vel_right * std::fabsf(vel_right) +
		(30.0f * p_motion_props->linear_damping) * vel_right;
	resistance_lat += 0.95f * p_motion_props->mass * 10.0f * vel_right;

	// Calculate forces
	forward_force = -resistance_long;
	right_force = -resistance_lat;

	// turning angle
	Float32 steering_angle{ glm::radians(max_angular_speed * 12.0f / speed) };
	Float32 angle_change{ (speed * std::sinf(steering_angle) / 4.0f) * dt };

	// calculate angle to target
	Float32 dot = glm::dot(final_dir, local_forward);
	Vec3 cross = glm::cross(final_dir, local_forward);
	Float32 angle_to_target = atan2f(glm::dot(cross, Vec3{ 0.0f, 1.0f, 0.0f }), dot);
	angle_to_target = glm::degrees(angle_to_target);

	CheckSameSpot(dt);

	if (in_same_spot) {
		reverse_timer -= dt;
		// back up straight for 1.33 seconds...
		forward_force -= max_reverse_speed;
		// ... then turn
		if (reverse_timer < 0.66f) {
			// turn
			move_angle -= turn_direction * angle_change;
		}

		if (reverse_timer <= 0.0f) {
			reverse_timer = 2.0f;
		}
	}
	else {
		// angle between -115 and +115 degrees
		// move forward
		if (angle_to_target > -115.0f &&
			angle_to_target < 115.0f) {
			forward_force += max_forward_speed;

			// turning
			if (speed > 1.0f) {
				// target on left
				if (angle_to_target > 0.0f) {
					move_angle -= angle_change;
				}
				// target on right
				else {
					move_angle += angle_change;
				}
			}
		}
		// else target is behind us
		// move backward
		else {
			// braking
			if (vel_forward > 0.0f) {
				forward_force -= max_forward_speed;
			}
			// reversing
			else {
				forward_force -= max_reverse_speed;

				// turning
				if (speed > 1.0f) {
					// target on left
					if (angle_to_target > 135.0f) {
						move_angle -= angle_change;
					}
					// target on right
					else if (angle_to_target < -135.0f) {
						move_angle += angle_change;
					}
				}
			}
		}
	}

	// clamp move angle and update rigidbody
	move_angle = glm::clamp(move_angle, -3.142f, 3.142f);
	p_rigidbody->orientation = Quat(Vec3{ 0.0f, move_angle, 0.0f });;

	p_rigidbody->AddForce(forward_force * local_forward + right_force * local_right);
}

Bool GenericCarEnemy::CloseEnough(Vec3 const& loc1, Vec3 const& loc2) {
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	return fabsf(loc1.x - loc2.x) < p_rb->local_bounds.halfwidths.z &&
			fabsf(loc1.z - loc2.z) < p_rb->local_bounds.halfwidths.z;
}

void GenericCarEnemy::CheckSameSpot(Float32 dt) {
	RigidBody* p_rb = GetOwner()->HasComponent<RigidBody>();

	if (in_same_spot_timer <= 0.0f) {
		in_same_spot_timer = 3.0f;

		in_same_spot = CloseEnough(p_rb->position, prev_pos);

		prev_pos = p_rb->position;

		// set turn direction
		if (rand_float.get()->Gen() > 0.5f) {
			turn_direction = 1;
		}
		else {
			turn_direction = -1;
		}
	}

	in_same_spot_timer -= dt;
}

void GenericCarEnemy::HealthUpdate(Int32 damage) {
	GameObject* owner = GetOwner();
	Health* p_health = owner->HasComponent<Health>();
	SIK_ASSERT(p_health, "Should have Health.");

	p_health->TakeDamage(damage);

	Int32 curr_hp = p_health->GetCurrHP();
	Int32 max_hp = p_health->GetMaxHP();

	Float32 curr_health_ratio = static_cast<Float32>(curr_hp) / static_cast<Float32>(max_hp);

	MeshRenderer* mr = owner->HasComponent<MeshRenderer>();

	switch (enemy_type)
	{
		break; case EnemyType::SmallCar:
		{
			// change model
			if (curr_health_ratio >= 1.0f) {
				mr->mesh = p_resource_manager->GetMesh("muscle_car.fbx");

				// emitters
				p_smoke_emitter->is_active = false;
				p_flame_emitter->is_active = false;
			}
			else if (curr_health_ratio < 1.0f &&
					 curr_health_ratio >= 0.5f) {
				mr->mesh = p_resource_manager->GetMesh("muscle_car_dmg_50.fbx");

				// emitters
				p_smoke_emitter->is_active = true;
				p_flame_emitter->is_active = false;

				// Hit sound effect
				p_audio_manager->PlayAudio("TURRET_IMP"_sid,
					p_audio_manager->sfx_chanel_group,
					p_audio_manager->turret_impact_vol,
					1.5f,
					false,
					0);
			}
			else if (curr_health_ratio < 0.5f &&
					 curr_health_ratio >= 0.0f) {
				mr->mesh = p_resource_manager->GetMesh("muscle_car_dmg_0.fbx");

				// enable emitters
				p_smoke_emitter->is_active = true;
				p_flame_emitter->is_active = true;

				// other statuses
				is_chasing = false;
				is_alive = false;

				p_base_state->GetGamePlayState()->DecrementSmallCarCount();

				// Play destroyed sound effect
				p_audio_manager->PlayAudio("TURRET_DES"_sid,
								p_audio_manager->sfx_chanel_group,
								p_audio_manager->turret_destroy_vol,
								1.5f,
								false,
								0);
			}
		}
		break; case EnemyType::BigCar:
		{
			// change model
			if (curr_health_ratio >= 1.0f) {
				mr->mesh = p_resource_manager->GetMesh("monster_truck.fbx");

				// emitters
				p_smoke_emitter->is_active = false;
				p_flame_emitter->is_active = false;
			}
			else if (curr_health_ratio < 1.0f &&
				curr_health_ratio >= 0.66f) {
				mr->mesh = p_resource_manager->GetMesh("monster_truck_dmg_50.fbx");

				// emitters
				p_smoke_emitter->is_active = false;
				p_flame_emitter->is_active = false;

				// Hit sound effect
				p_audio_manager->PlayAudio("TURRET_IMP"_sid,
					p_audio_manager->sfx_chanel_group,
					p_audio_manager->turret_impact_vol,
					1.5f,
					false,
					0);
			}
			else if (curr_health_ratio < 0.66f &&
				curr_health_ratio >= 0.33f) {
				mr->mesh = p_resource_manager->GetMesh("monster_truck_dmg_50.fbx");

				// emitters
				p_smoke_emitter->is_active = true;
				p_flame_emitter->is_active = false;

				// Hit sound effect
				p_audio_manager->PlayAudio("TURRET_IMP"_sid,
					p_audio_manager->sfx_chanel_group,
					p_audio_manager->turret_impact_vol,
					1.5f,
					false,
					0);
			}
			else if (curr_health_ratio < 0.33f &&
				curr_health_ratio >= 0.0f) {
				mr->mesh = p_resource_manager->GetMesh("monster_truck_dmg_0.fbx");

				// emitters
				p_smoke_emitter->is_active = true;
				p_flame_emitter->is_active = true;

				// other statuses
				is_chasing = false;
				is_alive = false;

				p_base_state->GetGamePlayState()->DecrementBigCarCount();

				// Play destroyed sound effect
				p_audio_manager->PlayAudio("TURRET_DES"_sid,
					p_audio_manager->sfx_chanel_group,
					p_audio_manager->turret_destroy_vol,
					1.5f,
					false,
					0);
			}
		}
	}
}

void GenericCarEnemy::SetupEmitters() {
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

void GenericCarEnemy::UpdateHeadlightPositions() {
	Transform* p_tr = GetOwner()->HasComponent<Transform>();

	p_headlights[0]->position = p_tr->LocalToWorld(Vec3(0.5 * headlights_width, headlights_vertical_offset, -(headlights_forward_offset + (p_headlights[0]->length / 2))));
	p_headlights[1]->position = p_tr->LocalToWorld(Vec3(-0.5 * headlights_width, headlights_vertical_offset, -(headlights_forward_offset + (p_headlights[0]->length / 2))));

	p_headlights[0]->orientation = p_headlights[1]->orientation = p_tr->orientation;
}

void GenericCarEnemy::SetHeadlights(Bool headlights_on) {
	p_headlights[0]->enabled = headlights_on;
	p_headlights[1]->enabled = headlights_on;
}

BEGIN_ATTRIBUTES_FOR(GenericCarEnemy)
	DEFINE_MEMBER(Float32, max_forward_speed)
	DEFINE_MEMBER(Float32, max_reverse_speed)
	DEFINE_MEMBER(Float32, max_angular_speed)
	DEFINE_MEMBER(Float32, aggro_radius)
	DEFINE_MEMBER(Int32, damage)
	DEFINE_MEMBER(Float32, destroy_vel_threshold)
	DEFINE_MEMBER(Float32, headlights_forward_offset)
	DEFINE_MEMBER(Float32, headlights_vertical_offset)
	DEFINE_MEMBER(Float32, headlights_width)
	DEFINE_MEMBER(Float32, headlights_radius)
	DEFINE_MEMBER(Float32, headlights_length)
	DEFINE_MEMBER(Vec3, headlights_color)
END_ATTRIBUTES