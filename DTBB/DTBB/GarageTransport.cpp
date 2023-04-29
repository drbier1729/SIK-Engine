#include "stdafx.h"

#include "Engine/InputAction.h"

#include "GarageTransport.h"
#include "BaseState.h"
#include "CarController.h"
#include "GamePlayState.h"
#include "FadeState.h"

#include "Engine/GraphicsManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GameObject.h"
#include "Engine/Transform.h"
#include "Engine/GameStateManager.h"
#include "Engine/InputManager.h"
#include "Engine/Behaviour.h"

Vector<GUIObject*> GarageTransport::gui_objects;

GarageTransport::GarageTransport() : 
	transport_radius(0.0f), can_transport(false), p_light(p_graphics_manager->AddLocalLight()),
	enabled(true), light_color(1.0f) {
	
	if (gui_objects.size() == 0) {
		gui_objects = p_gui_object_manager->CreateGUIFromFile("garage_prompt.json");
		for (auto& gui_obj : gui_objects)
			gui_obj->Disable();
	}
}

GarageTransport::~GarageTransport() noexcept {
	p_graphics_manager->RemoveLocalLight(p_light);
}

void GarageTransport::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<GarageTransport>(json_value, this);
}

void GarageTransport::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<GarageTransport>(json_value, this, alloc);
}

void GarageTransport::Link() {

}

void GarageTransport::Update(Float32 dt) {
	CheckPlayerPosition();
	UpdateUIPrompt(dt);

	if (can_transport) {
		GameObject* player_go = p_base_state->GetPlayerGameObjPtr();
		CarController* player_controller = player_go->HasComponent<CarController>();
		InputAction& player_action_map = player_controller->GetActionMap();

		if (player_action_map.IsActionTriggered(InputAction::Actions::ACTION_4)) {
			//Transport back to garage.
			GamePlayState* curr_state = p_base_state->GetGamePlayState();
			curr_state->EnterGarage();
		}
	}
}

void GarageTransport::Disable() {
	enabled = false;
	for (auto& gui_obj : gui_objects)
		gui_obj->Disable();

	p_light->enabled = false;
}

void GarageTransport::Enable() {
	enabled = true;

	Transform* owner_tr = GetOwner()->HasComponent<Transform>();
	p_light->color = light_color / Vec3(255.0f);
	p_light->radius = transport_radius;
	p_light->position = owner_tr->position;
	p_light->ease_out = false;
	p_light->enabled = true;
}

void GarageTransport::CheckPlayerPosition() {
	GameObject* player_go = p_base_state->GetPlayerGameObjPtr();
	Transform* player_tr = player_go->HasComponent<Transform>();

	Transform* owner_tr = GetOwner()->HasComponent<Transform>();
	Vec3 diff_vector = owner_tr->position - player_tr->position;
	if (glm::length(diff_vector) < transport_radius) {
		can_transport = true;
		p_light->color = (light_color / Vec3(255.0f)) * 1.5f;
	}
	else {
		can_transport = false;
		p_light->color = light_color / Vec3(255.0f);
	}
}

void GarageTransport::UpdateUIPrompt(Float32 dt) {
	if (can_transport) {
		for (auto& gui_obj : gui_objects)
			gui_obj->Enable();
	}
	else {
		for (auto& gui_obj : gui_objects)
			gui_obj->Disable();
	}

	for (auto& gui_obj : gui_objects)
		gui_obj->Update(dt);
}

BEGIN_ATTRIBUTES_FOR(GarageTransport)
DEFINE_MEMBER(Float32, transport_radius)
DEFINE_MEMBER(Vec3, light_color)
END_ATTRIBUTES