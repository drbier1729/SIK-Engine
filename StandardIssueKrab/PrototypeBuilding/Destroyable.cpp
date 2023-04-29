#include "stdafx.h"
#include "Destroyable.h"
#include "Engine/AudioManager.h"
#include "Engine/GameObject.h"
#include "Engine/ResourceManager.h"
#include "Engine/MeshRenderer.h"

#include "PlayerController.h"

template<class T>
void SwapPointers(T* p1, T* p2) {
	T* t_p = p1;
	p1 = p2;
	p2 = t_p;
}

void Destroyable::Deserialize(rapidjson::Value const& json_value) {
	const char* alt_tex_name = json_value.FindMember("AltTexture")->value.GetString();
	alt_tex = p_resource_manager->GetTexture(alt_tex_name);

	alt_mesh = p_resource_manager->GetMesh("Plane");
}

void Destroyable::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
}

void Destroyable::OnCollide(GameObject* other) {
	PlayerController* p_c = other->HasComponent<PlayerController>();
	RigidBody* rb = GetOwner()->HasComponent<RigidBody>();
	if (not rb->IsEnabled())
		return;

	if (p_c) {
		if (p_c->IsTopSpeed()) {
			destroyed = true;
			
			rb->Enable(false);

			//Set the alternate texture
			MeshRenderer* mr = GetOwner()->HasComponent<MeshRenderer>();
			SwapPointers(mr->material->base_color, alt_tex);

			//Set alternate mesh
			SwapPointers(mr->mesh, alt_mesh);
			mr->enabled = false;

			p_audio_manager->PlaySoundEffect("BREAK");
		}
	}
}

void Destroyable::Update(Float32 dt) {
	if (not destroyed_this_frame && destroyed) {
		SIK_INFO("Destroying destroyable");
		destroyed_this_frame = true;
		destroyed = false;
	}
	else {
		destroyed_this_frame = false;
	}
}

bool Destroyable::isDestroyed() const {
	return destroyed;
}

bool Destroyable::isDestroyedThisFrame() const {
	return destroyed_this_frame;
}
