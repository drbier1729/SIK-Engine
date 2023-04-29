#include "stdafx.h"
#include "Destroyable.h"
#include "Engine/AudioManager.h"
#include "Engine/GameObject.h"
#include "Engine/ResourceManager.h"
#include "Engine/MeshRenderer.h"
#include "Engine/MotionProperties.h"
#include "Engine/Serializer.h"
#include "Engine/ParticleSystem.h"
#include "Engine/InputManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/Factory.h"
#include "GamePlayState.h"
#include "BaseState.h"
#include "Collectable.h"

template<class T>
void SwapPointers(T* p1, T* p2) {
	T* t_p = p1;
	p1 = p2;
	p2 = t_p;
}

Destroyable::Destroyable():
	destroyed{ false },
	p_hit_emitter{ p_particle_system->NewEmitter() },
	destroyed_this_frame{ false },
	alt_tex{ nullptr },
	alt_mesh{ nullptr },
	destroy_momentum{ 0.0f },
	starting_position{ 0.0f },
	starting_orientation{},
	starting_tex{nullptr},
	starting_mesh{nullptr}
{
	// bullet hit emitter settings
	p_hit_emitter->particles_per_sec = 25;
	p_hit_emitter->pos_gen_bnds = {
		.min = Vec3{ 0.0f, 0.0f, 0.0f },
		.max = Vec3{ 0.0f, 0.0f, 0.0f }
	};
	p_hit_emitter->color_gen_bnds = {
		.min = Vec4{ 0.7f, 0.6f, 0.6f, 1.0f },
		.max = Vec4{ 0.7f, 0.6f, 0.6f, 1.0f }
	};
	p_hit_emitter->speed_gen_bnds = {
		.min = 22.0f,
		.max = 22.0f
	};
	p_hit_emitter->theta = {
		.min = glm::radians(-180.0f),
		.max = glm::radians(180.0f)
	};
	p_hit_emitter->phi = {
		.min = glm::radians(-150.0f),
		.max = glm::radians(-30.0f)
	};
	p_hit_emitter->lifetime_secs_gen_bnds = {
		.min = 0.3f,
		.max = 0.5f
	};
	p_hit_emitter->uniform_scale_gen_bnds = {
		.min = 0.2f,
		.max = 0.5f
	};
}

Destroyable::~Destroyable() noexcept {
	p_particle_system->EraseEmitter(p_hit_emitter);
}

void Destroyable::Deserialize(rapidjson::Value const& json_value) {
	const char* alt_tex_name = json_value.FindMember("AltTexture")->value.GetString();
	alt_tex = p_resource_manager->GetTexture(alt_tex_name);

	alt_mesh = p_resource_manager->GetMesh("Plane");

	DeserializeReflectable<Destroyable>(json_value, this);
}

void Destroyable::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Destroyable>(json_value, this, alloc);
}

void Destroyable::Link() {
	RigidBody* owner_rb = GetOwner()->HasComponent<RigidBody>();
	starting_position = owner_rb->position;
	starting_orientation = owner_rb->orientation;

	MeshRenderer* mr = GetOwner()->HasComponent<MeshRenderer>();
	starting_mesh = mr->mesh;
	starting_tex = mr->material->base_color;
}

void Destroyable::OnCollide(GameObject* other) {
	if (destroyed)
		return;

	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (not rb || not rb->motion_props || not rb->IsEnabled())
		return;

	// Don't get destroyed by flying collectables
	if (other->HasComponent<Collectable>())
		return;

	RigidBody* other_rb = other->HasComponent<RigidBody>();
	if (!(other_rb && other_rb->motion_props)) { return; }

	Vec3 owner_velocity = rb->motion_props->linear_velocity;
	Vec3 other_velocity = other_rb->motion_props->linear_velocity;
	Float32 relative_speed = glm::length(owner_velocity - other_velocity);

	Float32 curr_momentum = relative_speed * other_rb->motion_props->mass;

	if (curr_momentum > destroy_momentum) {
		SayGoodbyeAndDie();
	}
}

void Destroyable::Update(Float32 dt) {
	if (not destroyed_this_frame && to_destroy) {
		destroyed_this_frame = true;
		to_destroy = false;
	}
	else {
		destroyed_this_frame = false;
	}

	if (destroyed) {
		// Disable the RigidBody
		RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
		rb->Enable(false);

		//Set the alternate texture
		MeshRenderer* mr = GetOwner()->HasComponent<MeshRenderer>();
		mr->enabled = false;
		//If all the particles have finished emitting, destroy this object
		if (p_hit_emitter->particles.size() == 0) {
			auto* gameplay_state = p_base_state->GetGamePlayState();
			GetOwner()->Disable();
			gameplay_state->RemoveObject(GetOwner());
		}
	}
}

bool Destroyable::isDestroyed() const {
	return destroyed;
}

bool Destroyable::isDestroyedThisFrame() const {
	return destroyed_this_frame;
}

void Destroyable::Enable() {
	destroyed_this_frame = false;
	destroyed = false;
	to_destroy = false;
}

void Destroyable::Reset() {
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!rb) { return; }
	rb->position = starting_position;
	rb->orientation = starting_orientation;
	destroyed_this_frame = false;
	destroyed = false;
	to_destroy = false;

	if (!rb->motion_props) { return; }
	rb->motion_props->linear_velocity = Vec3(0);

	MeshRenderer* mr = GetOwner()->HasComponent<MeshRenderer>();
	if (mr->mesh != starting_mesh) {
		SwapPointers(mr->mesh, alt_mesh);
	}

	if (mr->material->base_color != starting_tex) {
		SwapPointers(mr->material->base_color, alt_tex);
	}
}


void Destroyable::SayGoodbyeAndDie() {
	to_destroy = true;
	destroyed = true;

	// Disable the RigidBody
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (!rb) { return; }
	rb->Enable(false);

	//Set the alternate texture
	MeshRenderer* mr = GetOwner()->HasComponent<MeshRenderer>();
	SwapPointers(mr->material->base_color, alt_tex);

	//Set alternate mesh
	SwapPointers(mr->mesh, alt_mesh);
	mr->enabled = false;

	// Emitter
	p_hit_emitter->position = rb->position;
	p_hit_emitter->EmitParticles(25);

	// Playing the destroying sound effect
	p_audio_manager->PlayAudio("IMPACT"_sid,
		p_audio_manager->sfx_chanel_group,
		p_audio_manager->wood_destroy_vol,
		1.5,
		false,
		0);

	// Rumbling controller on destroying objects
	p_input_manager->RumbleController(1000, 10000, 500);

	// Spawn Collectable object here
	GameObject* p_col_obj = p_factory->BuildGameObject("CollectableObject.json");
	if (not p_col_obj) { return; }

	RigidBody* col_rb = p_col_obj->HasComponent<RigidBody>();
	if (col_rb && col_rb->motion_props) {
		col_rb->motion_props->prev_position = rb->position;
		col_rb->position = rb->position;
		col_rb->Enable(true);

		Collectable* col = p_col_obj->HasComponent<Collectable>();
		if (col)
		{
			col->SetStartHeight(col_rb->position.y);
			col->SetTrailColor(Vec3{ 0.95f, 0.74f, 0.12f });
		}
	}

	// Update the game play state
	auto* gameplay_state = p_base_state->GetGamePlayState();
	gameplay_state->AddObject(p_col_obj);
	gameplay_state->DecrementDestructibleCount();
}

BEGIN_ATTRIBUTES_FOR(Destroyable)
DEFINE_MEMBER(Float32, destroy_momentum)
END_ATTRIBUTES