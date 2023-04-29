#include "stdafx.h"

#include "Engine/GameObject.h"
#include "Engine/GUIObject.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GameObjectManager.h"

#include "PromptState.h"
#include "BaseState.h"
#include "CarController.h"
#include "GamePlayState.h"
#include "ScriptInterface.h"
#include "TurretEnemy.h"
#include "Attachment.h"

PromptState::PromptState() : GameState(), car_amount_drifted(0), car_amount_moved(0),
	scripts_registered(false), p_junkyard_state(nullptr) {
	dialog_objects = p_gui_object_manager->CreateGUIFromFile("tutorial_gui.json");
}

PromptState::~PromptState() {
}

void PromptState::Enter() {
	for (auto& obj : dialog_objects)
		obj->Enable();

	if (p_junkyard_state) {
		p_junkyard_state->Enter();
	}

	if (not scripts_registered) {
		scripts_registered = true;
		RegisterPlayerToScripts();
	}

	// get position of the turret in the junkyard arena
	tutorial_turret_pos = GetTurretPosition();

	for (auto& gui_object : dialog_objects) {
		gui_object->DisableRender();
	}
}

void PromptState::Exit() {
	for (auto& obj : dialog_objects)
		obj->Disable();

	if (p_junkyard_state) {
		p_junkyard_state->Exit();
	}
}

void PromptState::Update(Float32 dt) {
	p_junkyard_state->Update(dt);
	GameState::Update(dt);
	UpdateCarStats(dt);
	DialogUpdate(dt);
}

void PromptState::FixedUpdate(Float32 fixed_timestep) {
	p_junkyard_state->FixedUpdate(fixed_timestep);
	GameState::FixedUpdate(fixed_timestep);
}

Float32 PromptState::GetCarAmountMoved() {
	return car_amount_moved;
}

Float32 PromptState::GetCarAmountDrifted() {
	return car_amount_drifted;
}

Float32 PromptState::GetDistanceToTurret() {
	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();
	RigidBody* player_rb = p_player_go->HasComponent<RigidBody>();

	return glm::length(player_rb->position - tutorial_turret_pos);
}

void PromptState::SetJunkyardState(GamePlayState* _p_junkyard_state) {
	p_junkyard_state = _p_junkyard_state;
}

void PromptState::DialogUpdate(Float32 dt) {
	p_gui_object_manager->UpdateUIControls(dt);
	for (auto& obj : dialog_objects)
		obj->Update(dt);
}

void PromptState::RegisterPlayerToScripts() {
	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();
	for (auto& game_object : objects_in_current_state) {
		Behaviour* bh = game_object->HasComponent<Behaviour>();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaPromptState(script.script_state, this);
			}
		}
	}

	for (auto& gui_object : dialog_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaGamePlayState(script.script_state, p_junkyard_state);

				RegisterLuaPromptState(script.script_state, this);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {
					RegisterLuaPlayerObject(script.script_state,
						p_player_go, "player_object");

					RegisterLuaGamePlayState(script.script_state, p_junkyard_state);

					RegisterLuaPromptState(script.script_state, this);
				}
			}
		}
	}
}

void PromptState::UpdateCarStats(Float32 dt) {
	GameObject* player_go = p_base_state->GetPlayerGameObjPtr();
	CarController* player_cc = player_go->HasComponent<CarController>();
	
	//If the car is moving update the move amount
	if (player_cc->GetCurrentCarSpeed() > 5.0f) {
		car_amount_moved += dt;

		//If the car is drifting update the drift amount
		if (player_cc->IsDrifting()) {
			car_amount_drifted += dt;
		}
	}
}

Vec3 PromptState::GetTurretPosition() {
	// find turret enemy present in the tutorial
	for (auto&& object : p_junkyard_state->GetObjectsInState()) {
		// find turret enemy
		if (object->HasComponent<TurretEnemy>()) {
			// get turret base from turret enemy
			Attachment* attach = object->HasComponent<Attachment>();
			GameObject* turret_base = attach->GetAttachedTo();

			// get position
			RigidBody* turret_base_rb = turret_base->HasComponent<RigidBody>();
			return turret_base_rb->position;
		}
	}
	return Vec3(0);
}

void PromptState::HandleEvent(SDL_Event const& e)
{
	if (p_junkyard_state) {
		p_junkyard_state->HandleEvent(e);
	}
}
