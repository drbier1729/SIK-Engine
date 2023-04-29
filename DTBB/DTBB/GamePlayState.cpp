#include "stdafx.h"

#include "Engine/GraphicsManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/RenderCam.h"
#include "Engine/AudioManager.h"
#include "Engine/GameObject.h"
#include "Engine/GUIObject.h"
#include "Engine/InputManager.h"
#include "Engine/GameStateManager.h"
#include "Engine/MemoryManager.h"
#include "Engine/GameObjectManager.h"

#include "BaseState.h"
#include "ScriptInterface.h"
#include "GamePlayState.h"
#include "VersusState.h"
#include "StartState.h"
#include "FollowCam.h"
#include "Destroyable.h"
#include "TurretEnemy.h"
#include "Health.h"
#include "FadeState.h"
#include "MenuState.h"
#include "GarageState.h"
#include "Health.h"
#include "Collectable.h"
#include "GenericCarEnemy.h"

GamePlayState::GamePlayState(const char* scene_json, StringID background_music, Bool _can_lose) :
	GameState(scene_json), 
	camera(p_graphics_manager->GetPActiveCam()), 
	turret_counter(0), destructible_counter(0),
	small_car_counter{ 0 }, big_car_counter{ 0 },
	start_menu_shown(false), from_pause(false),
	scripts_registered(false), can_lose(_can_lose),
	p_fade_out(nullptr), p_fade_in(nullptr), fading(false),
	p_lose_fade(nullptr),
	p_pause_state(p_memory_manager->GetDefaultAllocator().new_object<MenuState>("pause_menu.json")),
	music_tag(background_music), music_id(0),
	is_last_level{false},
	current_progress{ Progress::Start }, prev_progress{ Progress::Start } {
	
	hud_objects = p_gui_object_manager->CreateGUIFromFile("HUD.json");
	p_fade_out = p_memory_manager->GetDefaultAllocator().new_object<FadeState>("load_fade.json", 1.5f, true);
	p_fade_in = p_memory_manager->GetDefaultAllocator().new_object<FadeState>("load_fade.json", 1.5f, false);
	p_lose_fade = p_memory_manager->GetDefaultAllocator().new_object<FadeState>("lose_fade.json", 3.0f, false);


	ResetEnemyCountForHUD();

	p_pause_state->Exit();
	p_fade_in->Exit();
	p_fade_out->Exit();
	p_lose_fade->Exit();
}

GamePlayState::~GamePlayState() {
	p_memory_manager->GetDefaultAllocator().delete_object(p_fade_in);
	p_memory_manager->GetDefaultAllocator().delete_object(p_fade_out);
	p_memory_manager->GetDefaultAllocator().delete_object(p_pause_state);
	p_memory_manager->GetDefaultAllocator().delete_object(p_lose_fade);

	hud_objects.clear();
}

void GamePlayState::Enter() {
	for (auto& obj : objects_in_current_state) {
		obj->Enable();
	}

	for (auto& obj : hud_objects) {
		obj->Enable();
	}

	p_base_state->EnableAllObjects();
	
	if (from_pause) {
		from_pause = false;
		p_audio_manager->background_music_chanel_group->setVolume(1.0f);
		return;
	}
	music_id = p_audio_manager->PlayAudio(music_tag,
		p_audio_manager->background_music_chanel_group,
		p_audio_manager->construction_arena_back_vol,
		1.0f,
		false,
		-1);

	// camera settings
	camera->SetPosition(Vec3{ 10.f, 50.0f, 0.0f });
	camera->SetControllerMode(ControlMode::ISOMETRIC);
	camera->SetOrthoSize(35.0f);
	camera->SetZoom(40.0f);
	
	if (not scripts_registered) {
		scripts_registered = true;
		RegisterPlayerToScripts();
	}

	if (current_progress == Progress::Start) {
		SetProgress(Progress::Working);
	}

	p_fade_out->Enter();
	fading = false;
}

void GamePlayState::Exit() {
	for (auto& obj : hud_objects) {
		obj->Disable();
	}

	if (not from_pause) {
		p_audio_manager->Stop(music_id);
		SetProgress(current_progress);
	}

	p_fade_out->Exit();
	p_fade_in->Exit();
	p_lose_fade->Exit();
}

void GamePlayState::Update(Float32 dt) {
	HandleRemovedObjects();

	if (current_progress == Progress::Lose) {
		p_lose_fade->Update(dt);
		if (p_lose_fade->IsComplete()) {
			//Setting false to say we are enter garage without a fade since we already have a death fade.
			EnterGarage(false);
		}
		return;
	}

	if (fading) {
		if (not p_fade_in->IsComplete()) {
			p_fade_in->Update(dt);
		}
		else {
			fading = false;
			//p_fade_in->Exit();
			//Fade is complete, enter the garage without a fade
			EnterGarage(false);
		}
		return;
	}

	p_base_state->Update(dt);
	GameState::Update(dt);

	HUDUpdate(dt);
	ShadowFollowPlayer();
	
	//Check pause
	if (p_gui_object_manager->gui_action_map.IsActionTriggered(
		InputAction::Actions::ACTION_START)) {
		PauseGame();
	}

	if (not p_fade_out->IsComplete())
		p_fade_out->Update(dt);

	if (can_lose)
		UpdateGameCondition();
}

void GamePlayState::FixedUpdate(Float32 fixed_timestep) {
	if (current_progress == Progress::Lose) {
		return;
	}

	p_base_state->FixedUpdate(fixed_timestep);
	GameState::FixedUpdate(fixed_timestep);
}

void GamePlayState::DisableAllGameObjects() {
	for (auto& obj : objects_in_current_state) {

		obj->Disable();
	}
}

void GamePlayState::SetProgress(Progress n) {
	prev_progress = current_progress;
	current_progress = n;
}

Progress GamePlayState::GetProgress() const {
	return current_progress;
}

Progress GamePlayState::GetPreviousProgress() const {
	return prev_progress;
}

void GamePlayState::IncrementTurretCount() {
	turret_counter++;
	SetProgress(Progress::Working); // new enemies
}
void GamePlayState::DecrementTurretCount() {
	turret_counter--;
	if (turret_counter + small_car_counter + big_car_counter <= 0) {
		SetProgress(Progress::Complete);
	}
}
Int32 GamePlayState::GetTurretCount() const {
	return turret_counter;
}

void GamePlayState::IncrementDestructibleCount() {
	destructible_counter++;
}
void GamePlayState::DecrementDestructibleCount() {
	destructible_counter--;
}
Int32 GamePlayState::GetDestructibleCount() const {
	return destructible_counter;
}


void GamePlayState::IncrementSmallCarCount() {
	small_car_counter++;
	SetProgress(Progress::Working); // new enemies
}
void GamePlayState::DecrementSmallCarCount() {
	small_car_counter--;
	if (turret_counter + small_car_counter + big_car_counter <= 0) {
		SetProgress(Progress::Complete);
	}
}
Int32 GamePlayState::GetSmallCarCount() const {
	return small_car_counter;
}

void GamePlayState::IncrementBigCarCount() {
	big_car_counter++;
	SetProgress(Progress::Working); // new enemies
}
void GamePlayState::DecrementBigCarCount() {
	big_car_counter--;
	if (turret_counter + small_car_counter + big_car_counter <= 0) {
		SetProgress(Progress::Complete);
	}
}
Int32 GamePlayState::GetBigCarCount() const {
	return big_car_counter;
}


void GamePlayState::EnterGarage(Bool with_fade) {
	if (with_fade) {
		fading = true;
		p_fade_in->Enter();
	}
	else {
		//Pop states until we get to the garage
		do {
			p_gamestate_manager->PopState();
		} while (dynamic_cast<GarageState*>(p_gamestate_manager->GetStackTop()) == nullptr);

		DisableAllGameObjects();
	}
}

void GamePlayState::PauseGame() {
	from_pause = true;

	p_base_state->DisableAllObjects();
	for (auto& obj : objects_in_current_state) {
		obj->DisableExceptRender();
	}
	p_gamestate_manager->PushState(p_pause_state);

	p_audio_manager->background_music_chanel_group->setVolume(0.3f);
}

FadeState* GamePlayState::GetFadeState() {
	return p_fade_out;
}

void GamePlayState::EnableHUDObjects() {
	for (auto& obj : hud_objects) {
		obj->Enable();
	}
	p_fade_out->Enter();
}

void GamePlayState::ResetLevel() {
	for (auto& obj : deleted_objects) {
		if (obj->HasComponent<Collectable>()) {
			//We don't want to reset collectables.
			p_game_obj_manager->DeleteGameObject(obj);
			continue;
		}

		objects_in_current_state.push_back(obj);
	}
	deleted_objects.clear();
	ResetAndEnableAllObjects();

	ResetEnemyCountForHUD();
	RegisterPlayerToScripts();
}

void GamePlayState::SetLastLevel(Bool _is_last_level) {
	is_last_level = _is_last_level;
}

Bool GamePlayState::IsLastLevel() {
	return is_last_level;
}

void GamePlayState::HUDUpdate(Float32 dt) {
	for (auto& ui_obj : hud_objects) {
		ui_obj->Update(dt);
	}
}

void GamePlayState::ShadowFollowPlayer() {
	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();
	Transform* tr = p_player_go->HasComponent<Transform>();
	p_graphics_manager->SetShadowLookTarget(
		Vec3(tr->position.x, 0, tr->position.z));
}

void GamePlayState::RegisterPlayerToScripts()  {
	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();

	for (auto& game_object : objects_in_current_state) {
		Behaviour* bh = game_object->HasComponent<Behaviour>();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaGamePlayState(script.script_state, this);
			}
		}
	}

	for (auto& gui_object : hud_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaGamePlayState(script.script_state, this);
			}
		}

		Vector<GUIObject*> embedded_objects = gui_object->GetEmbeddedObjectsRecursive();
		for (auto& em_obj : embedded_objects) {
			Behaviour* bh = em_obj->GetBehaviour();
			if (bh) {
				for (auto& script : bh->scripts) {
					RegisterLuaPlayerObject(script.script_state,
						p_player_go, "player_object");

					RegisterLuaGamePlayState(script.script_state, this);
				}
			}
		}
	}
}

void GamePlayState::CheckGarageEnter() {
	if (p_gui_object_manager->gui_action_map.IsActionTriggered(
		InputAction::Actions::ACTION_SELECT)) {
		EnterGarage();
	}
}

void GamePlayState::UpdateGameCondition() {
	if (current_progress != Progress::Working)
		return;

	Health* player_health = p_base_state->GetPlayerGameObjPtr()->HasComponent<Health>();
	if (player_health->GetCurrHP() <= 0) {
		current_progress = Progress::Lose;
		p_lose_fade->Enter();
	}
}

void GamePlayState::HandleRemovedObjects() {
	if (not removed_objects.empty()) {
		std::sort(removed_objects.begin(), removed_objects.end());
		std::sort(objects_in_current_state.begin(), objects_in_current_state.end());

		auto r_it = removed_objects.begin();
		std::erase_if(objects_in_current_state,
			[this, &r_it](GameObject* x) -> Bool {
				while (r_it != removed_objects.end() && *r_it < x) { ++r_it; }
				return (r_it != removed_objects.end() && *r_it == x);
			}
		);

		//Delete the removed game objects
		for (GameObject* obj : removed_objects) {
			if (obj->HasComponent<Collectable>())
				p_game_obj_manager->DeleteGameObject(obj);
			else
				deleted_objects.push_back(obj);
		}
		removed_objects.clear();
	}
}

void GamePlayState::ResetEnemyCountForHUD() {

	turret_counter = 0;
	small_car_counter = 0;
	big_car_counter = 0;
	destructible_counter = 0;

	for (auto& obj : objects_in_current_state) {
		obj->Enable();
		TurretEnemy* t_e = obj->HasComponent<TurretEnemy>();
		if (t_e != nullptr) {
			turret_counter++;
		}

		Destroyable* d_c = obj->HasComponent<Destroyable>();
		if (d_c != nullptr) {
			destructible_counter++;
		}

		GenericCarEnemy* c_e = obj->HasComponent<GenericCarEnemy>();
		if (c_e != nullptr) {
			if (c_e->GetEnemyType() == GenericCarEnemy::EnemyType::SmallCar) {
				small_car_counter++;
			}
			if (c_e->GetEnemyType() == GenericCarEnemy::EnemyType::BigCar) {
				big_car_counter++;
			}
		}
	}

}

void GamePlayState::HandleEvent(SDL_Event const& e) {

	if (e.type == SDL_WINDOWEVENT)
	{
		switch (e.window.event)
		{
			break; case SDL_WINDOWEVENT_MINIMIZED:
			{
				PauseGame();
			}
		}
	}

}
