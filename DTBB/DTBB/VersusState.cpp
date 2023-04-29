#include "stdafx.h"
#include "VersusState.h"

#include "Engine/MemoryManager.h"
#include "Engine/RenderCam.h"
#include "Engine/GraphicsManager.h"
#include "Engine/GameObject.h"
#include "Engine/GUIObject.h"
#include "Engine/Behaviour.h"
#include "Engine/GameManager.h"
#include "Engine/InputManager.h"
#include "Engine/GameStateManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/GUIObjectManager.h"

#include "ScriptInterface.h"
#include "BaseState.h"
#include "Health.h"
#include "FollowCam.h"

VersusState::VersusState() : GameState("VersusScene.json"),
	camera(nullptr), prev_camera(nullptr), p_player_2_go(nullptr) {
	camera = p_memory_manager->GetDefaultAllocator().new_object<RenderCam>(
		p_graphics_manager->GetWindowWidth(), p_graphics_manager->GetWindowHeight(),
		"versus_camera.lua", "camera_controls.json");

	hud_objects = p_gui_object_manager->CreateGUIFromFile("versus_HUD.json");
}

VersusState::~VersusState() {
	p_memory_manager->GetDefaultAllocator().delete_object(camera);

	for (auto& game_obj : objects_in_current_state) {
		p_game_obj_manager->DeleteGameObject(game_obj);
	}
}

void VersusState::Enter() {
	prev_camera = p_graphics_manager->GetPActiveCam();

	p_graphics_manager->SetPActiveCam(camera);

	camera->SetPosition(Vec3{ 10.f, 50.0f, 0.0f });
	camera->SetControllerMode(ControlMode::ISOMETRIC);
	camera->SetOrthoSize(35.0f);
	

	for (auto& game_obj : objects_in_current_state) {
		game_obj->Enable();

		if (game_obj->GetName().compare("GamePlayer_2") == 0)
			SetPlayerTwoGameObjPtr(game_obj);
	}

	for (auto& obj : hud_objects)
		obj->Enable();

	RegisterPlayerToScripts();

	FollowCam* fc = p_base_state->GetPlayerGameObjPtr()->HasComponent<FollowCam>();
	fc->SetEnable(false);
}

void VersusState::Exit() {
	if (prev_camera != nullptr)
		p_graphics_manager->SetPActiveCam(prev_camera);

	for (auto& obj : objects_in_current_state)
		obj->Disable();

	for (auto& obj : hud_objects)
		obj->Disable();
}

void VersusState::Update(Float32 dt) {
	CameraUpdate(dt);

	p_base_state->Update(dt);
	GameState::Update(dt);

	HUDUpdate(dt);

	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
		p_base_state->GetPlayerGameObjPtr()->HasComponent<Health>()->TakeDamage(-50);
		p_player_2_go->HasComponent<Health>()->TakeDamage(-50);
		p_gamestate_manager->PopState();
	}
}

void VersusState::FixedUpdate(Float32 fixed_timestep) {
	p_base_state->FixedUpdate(fixed_timestep);
	GameState::FixedUpdate(fixed_timestep);
}

GameObject* VersusState::GetPlayerTwoGameObjPtr() const {
	return p_player_2_go;
}

void VersusState::SetPlayerTwoGameObjPtr(GameObject* p_obj) {
	p_player_2_go = p_obj;
}

void VersusState::HUDUpdate(Float32 dt){
	for (auto& obj : hud_objects)
		obj->Update(dt);
}

void VersusState::RegisterPlayerToScripts() {
	for (auto& game_object : objects_in_current_state) {
		Behaviour* bh = game_object->HasComponent<Behaviour>();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_2_go, "player_two_object");
			}
		}
	}

	for (auto& gui_object : hud_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_2_go, "player_two_object");
			}
		}
	}

	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();
	for (auto& game_object : objects_in_current_state) {
		Behaviour* bh = game_object->HasComponent<Behaviour>();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");
			}
		}
	}

	for (auto& gui_object : hud_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");
			}
		}
	}
}

void VersusState::CameraUpdate(Float32 dt) {
	Transform* tr_2 = p_player_2_go->HasComponent<Transform>();
	Transform* tr_1 = p_base_state->GetPlayerGameObjPtr()->HasComponent<Transform>();

	Vec3 diff_vec = tr_2->position - tr_1->position;
	Float32 diff_distance = glm::length(diff_vec);

	Vec3 camera_pos = tr_2->position - (glm::normalize(diff_vec) * (diff_distance / 2));
	camera->SetCameraCenter(camera_pos);

	Float32 max_distance = 20.0f;
	Vec3 default_pos(10, 50, 0);
	if (diff_distance > max_distance) {
		Float32 scale_factor = (diff_distance - max_distance);
		Vec3 new_pos = default_pos * (scale_factor*10);
		camera->SetPosition(new_pos);
	}
	camera->Update(dt);
}
