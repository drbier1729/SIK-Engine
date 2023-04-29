#include "stdafx.h"

#include "PrototypeCombat.h"

#include "Engine/MemoryManager.h"
#include "Engine/RenderCam.h"
#include "Engine/GraphicsManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/ScriptingManager.h"
#include "Engine/Factory.h"
#include "Engine/RandomGenerator.h"
#include "Engine/GUIObjectManager.h"

#include <chrono>

#include "Turret.h"
#include "Player.h"
#include "Enemy.h"
#include "CameraFollow.h"

PrototypeCombat::PrototypeCombat():
	alloc{ p_memory_manager->GetCurrentAllocator() },
	camera{ p_graphics_manager->GetPActiveCam() },
	player_go{ nullptr },
	p_turret{ nullptr },
	enemy_wave{ 0 },
	rand_flt_gen{ nullptr }
{
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto val = std::chrono::duration_cast<std::chrono::milliseconds>(ms.time_since_epoch());
	Uint64 seed = val.count();
	rand_flt_gen = alloc.new_object<UniformRandFloat32>(seed);
}

PrototypeCombat::~PrototypeCombat() {
	alloc.delete_object(rand_flt_gen);
}

// registers game side component functions used by scripts
void PrototypeCombat::SetupPrototype() {
	// camera settings
	camera->SetPosition(Vec3{ 10.f, 50.0f, 0.0f });
	camera->SetControllerMode(ControlMode::ISOMETRIC);
	camera->SetOrthoSize(35.0f);

	// load GUI for prototype
	p_gui_object_manager->CreateGUIFromFile("CombatGUI.json");

	auto& game_objs = p_game_obj_manager->GetGameObjectContainer();
	for (auto range = game_objs.all(); !range.is_empty(); range.pop_front()) {
		GameObject& game_obj = range.front();
		// store ptr to player obj
		if (game_obj.HasComponent<Player>()) {
			player_go = &game_obj;
			// set camera to follow player
			CameraFollow* p_cam_follow = player_go->HasComponent<CameraFollow>();
			SIK_ASSERT(p_cam_follow != nullptr, "No camera follow component");
			p_cam_follow->p_cam = camera;
		}
		if (Behaviour* p_behaviour = game_obj.HasComponent<Behaviour>()) {
			// register turret functions
			if (Turret* p_turret_temp = game_obj.HasComponent<Turret>()) {
				p_turret = p_turret_temp;
				for (auto& script : p_behaviour->scripts) {
					script->script_state.set_function("SetFiring", &Turret::SetFiring, p_turret);
					p_scripting_manager->RegisterActionMap(script->script_state, p_turret->GetInputAction());
				}
			}
		}
	}

	SpawnEnemyWave();
}

void PrototypeCombat::UpdatePrototype() {
	if (not p_turret->HasEnemies()) {
		//SpawnEnemyWave();
	}
}

// HELPERS

void PrototypeCombat::SpawnEnemyWave() {
	++enemy_wave;

	Uint32 num_enemies{ 10 * enemy_wave };
	for (Uint32 i = 0; i < num_enemies; ++i) {
		GameObject* p_enemy = p_factory->BuildGameObject("CombatEnemy.json");
		RigidBody* p_enemy_rb = p_enemy->HasComponent<RigidBody>();
		p_enemy_rb->position = RandVec3(Vec3{ -75.0f, 3.5f, -75.0f }, Vec3{ 75.0f, 3.5f, 75.0f }, *rand_flt_gen);
		Enemy* p_enem_comp = p_enemy->HasComponent<Enemy>();
		p_enem_comp->player_obj = player_go;
		p_enem_comp->p_turret = p_turret;

		p_turret->AddEnemy(p_enemy);
	}
}