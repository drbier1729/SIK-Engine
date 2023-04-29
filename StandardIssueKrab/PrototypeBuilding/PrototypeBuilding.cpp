#include "stdafx.h"
#include "Engine/RenderCam.h"
#include "PrototypeBuilding.h"
#include "Engine/GameObject.h"
#include "Engine/MemoryResources.h"
#include "Engine/FixedObjectPool.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputManager.h"
#include "Engine/MemoryManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"
#include "Engine/ParticleSystem.h"
#include "Engine/ResourceManager.h"
#include "Engine/Factory.h"


PrototypeBuilding::PrototypeBuilding() :
	camera(p_graphics_manager->GetWindowWidth(), 
		   p_graphics_manager->GetWindowHeight(),
		    "building_camera_controls.json"),
	player_go(nullptr),
	player_particle_emitter(nullptr)
{
	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();
	prototype_action_map.reset(
		alloc.new_object<InputAction>("building_prototype_controls.json")
	);

	p_graphics_manager->SetPActiveCam(&camera);
}

PrototypeBuilding::~PrototypeBuilding() {
	PolymorphicAllocator alloc = p_memory_manager->GetCurrentAllocator();
	alloc.delete_object(prototype_action_map.release());

}


/*
* Sets the player_go pointer
* Returns: void
*/
void PrototypeBuilding::SetupGameObjects() {
	auto& go_container = p_game_obj_manager->GetGameObjectContainer();
	for (auto r = go_container.all(); not r.is_empty(); r.pop_front()) {
		GameObject & game_obj = r.front();
		if (game_obj.GetName().ends_with("_Player")) {
			player_go = &game_obj;
		}
	}

	player_particle_emitter = p_particle_system->NewEmitter();
	player_particle_emitter->particles_per_sec = 2000;
	player_particle_emitter->gravity_scale = 1.0f;
	player_particle_emitter->uniform_scale_gen_bnds = { .min = 0.02f, .max = 0.1f };
	player_particle_emitter->color_gen_bnds = 
	{   .min = Vec4(238.0f / 255, 48.0f / 255, 28.0f / 255, 0),
		.max = Vec4(238.0f / 255, 178.0f / 255, 28.0f / 255, 0) };
}

void PrototypeBuilding::TeardownGameObjects() {
}

void PrototypeBuilding::HandleDestroyedObjects() {
	auto& go_container = p_game_obj_manager->GetGameObjectContainer();
	for (auto r = go_container.all(); not r.is_empty(); r.pop_front()) {
		GameObject& game_obj = r.front();
		Destroyable* destroyable_comp = game_obj.HasComponent<Destroyable>();
		if (destroyable_comp) {
			if (destroyable_comp->isDestroyedThisFrame()) {
				SpawnCollectable(game_obj.HasComponent<Transform>()->position);
			}
		}
	}
}

/*
* Creates a collectable game object at the specified location
* Returns: void
*/
void PrototypeBuilding::SpawnCollectable(Vec3 spawn_pos) {
	GameObject* collectable_obj = p_factory->BuildGameObject("CollectableObj.json");
	
	Transform* obj_t = collectable_obj->HasComponent<Transform>();
	obj_t->position = spawn_pos;

	RigidBody* obj_rb = collectable_obj->HasComponent<RigidBody>();
	obj_rb->position = obj_t->position;
	obj_rb->Enable(false);

	Behaviour* obj_bh = collectable_obj->HasComponent<Behaviour>();
	obj_bh->SetStateVariable(3, "activate_time");
}

/*
* Handles displaying the inventory count
*/
void PrototypeBuilding::Update(Float32 dt) {
	camera.Update(dt);

	Inventory* player_inv = player_go->HasComponent<Inventory>();
	String count_str = "x0";
	count_str.replace(1, 1, std::to_string(player_inv->GetCollectableCount()));
	p_gui_object_manager->GetTextObject(1)->SetText(count_str);

	PlayerController* p_c = player_go->HasComponent<PlayerController>();
	String speed_str(std::to_string(int(p_c->GetForce())));
	p_gui_object_manager->GetTextObject(3)->SetText(speed_str);
	UpdatePlayerParticles();
}

/*
* Updates the particle emitter for the player to modify according 
* to the player movement
*/
void PrototypeBuilding::UpdatePlayerParticles() {
	PlayerController* p_c = player_go->HasComponent<PlayerController>();
	if (p_c->IsTopSpeed()) {
		player_particle_emitter->is_active = true;

		Transform* p_t = player_go->HasComponent<Transform>();
		Vec3 player_reverse_vec = glm::normalize(p_c->ForwardVector()) * Vec3(-1);
		player_particle_emitter->position = p_t->position + (player_reverse_vec * 2.0f);
		/*player_particle_emitter->vel_gen_bnds = 
			{ .min = Vec3(player_reverse_vec.x, -0.2, player_reverse_vec.z), 
			  .max = Vec3(player_reverse_vec.x, 0.2, player_reverse_vec.z) };*/
	}
	else {
		//Disable the particles if the player is not going top speed
		player_particle_emitter->is_active = false;
	}
}

