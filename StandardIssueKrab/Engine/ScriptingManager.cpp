#include "stdafx.h"

#include "ScriptingManager.h"

#include "InputManager.h"
#include "GameManager.h"
#include "LogManager.h"
#include "Behaviour.h"
#include "AudioManager.h"
#include "Transform.h"
#include "GraphicsManager.h"
#include "GameObject.h"
#include "InputAction.h"
#include "GUIObject.h"
#include "RenderCam.h"
#include "GUIText.h"
#include "GameStateManager.h"
#include "GUIObjectManager.h"
#include "Button.h"

Behaviour* ScriptingManager::CreateBehaviour() {
	Behaviour* new_behaviour = script_pool.emplace();
	behaviours_list.push_back(new_behaviour);

	return new_behaviour;
}

Behaviour* ScriptingManager::CreateBehaviour(GUIObject* gui_object) {
	Behaviour* new_behaviour = CreateBehaviour();
	new_behaviour->owner_gui = gui_object;

	return new_behaviour;
}

Behaviour* ScriptingManager::CreateBehaviour(RenderCam* cam_object) {
	Behaviour* new_behaviour = CreateBehaviour();
	new_behaviour->owner_cam = cam_object;

	return new_behaviour;
}

void ScriptingManager::DeleteBehaviour(Behaviour* p_behaviour) {
	auto find_iter = std::find(behaviours_list.begin(), behaviours_list.end(), p_behaviour);
	behaviours_list.erase(find_iter);
	if(p_behaviour->owner)
		p_behaviour->owner->behaviour = nullptr;
	
	if(p_behaviour->owner_gui)
		p_behaviour->owner_gui->SetBehaviour(nullptr);

	script_pool.erase(p_behaviour);
}

/*
* Calls clear on the underlying mem resource
* Returns: void
*/
void ScriptingManager::DeleteAllBehaviours() {
	script_pool.clear();
	behaviours_list.clear();
}

void ScriptingManager::RegisterGlobals(sol::state& state) const {
	// Input
	// TODO: figure out input actions
	state.set_function("IsKeyPressed", &InputManager::IsKeyPressed, p_input_manager);
	state.set_function("IsKeyTriggered", &InputManager::IsKeyTriggered, p_input_manager);
	state.set_function("IsKeyReleased", &InputManager::IsKeyReleased, p_input_manager);
	state.set_function("IsMouseButtonPressed", &InputManager::IsMouseButtonPressed, p_input_manager);
	state.set_function("IsMouseButtonTriggered", &InputManager::IsMouseButtonTriggered, p_input_manager);
	state.set_function("IsMouseButtonReleased", &InputManager::IsMouseButtonReleased, p_input_manager);
	state.set_function("IsControllerButtonPressed", &InputManager::IsControllerButtonPressed, p_input_manager);
	state.set_function("IsControllerButtonTriggered", &InputManager::IsControllerButtonTriggered, p_input_manager);
	state.set_function("IsControllerButtonReleased", &InputManager::IsControllerButtonReleased, p_input_manager);
	state.set_function("IsAxisPressedPositive", &InputManager::IsAxisPressedPositive, p_input_manager);
	state.set_function("IsAxisPressedNegative", &InputManager::IsAxisPressedNegative, p_input_manager);
	state.set_function("IsAxisTriggeredPositive", &InputManager::IsAxisTriggeredPositive, p_input_manager);
	state.set_function("IsAxisTriggeredNegative", &InputManager::IsAxisTriggeredNegative, p_input_manager);

	state.set_function("AreControllersConnected", &InputManager::AreControllersConnected, p_input_manager);

	// Audio
	state.set_function("MuteSfx", &AudioManager::MuteSfx, p_audio_manager);
	state.set_function("UnMuteSfx", &AudioManager::UnmuteSfx, p_audio_manager);
	state.set_function("MuteBackMusic", &AudioManager::MuteBackMusic, p_audio_manager);
	state.set_function("UnMuteBackMusic", &AudioManager::UnMuteBackMusic, p_audio_manager);
	state.set_function("IsMusicMute", &AudioManager::IsMusicMute, p_audio_manager);
	state.set_function("IsSfxMute", &AudioManager::IsSfxMute, p_audio_manager);

	// Game
	state.set_function("QuitGame", &GameManager::Quit, p_game_manager);

	// TODO: add more global functions
	state.set_function("ScriptLog", &LogManager::ScriptLog);

	// StateManagement
	auto state_manager_lua_obj = state["StateManager"].get_or_create<sol::table>();
	state_manager_lua_obj.set_function(
		"PopState", &GameStateManager::PopState, p_gamestate_manager);

	//GLM Variables
	/*auto lua_glm = state["lua_glm"].get_or_create<sol::table>();
	state.new_usertype<Vec2>("Vec2", "x", &Vec2::x, "y", &Vec2::y);
	state.new_usertype<Vec3>("Vec3", "x", &Vec3::x, "y", &Vec3::y, "z", &Vec3::z);*/

	state.set("timer", 0.0);
}

void ScriptingManager::RegisterGameObjectFunctions(sol::state& state, GameObject* p_game_obj) const {
	// Game Object
	state.set_function("EnableGameObject", &GameObject::Enable, p_game_obj);
	state.set_function("DisableGameObject", &GameObject::Disable, p_game_obj);

	// Transform functions
	Transform* go_transform = p_game_obj->HasComponent<Transform>();
	if (go_transform) {
		state.set_function("SetPosition", &Transform::SetPosition, go_transform);
		state.set_function("SetScale", &Transform::SetScale, go_transform);

		state.set_function("PositionX", &Transform::GetPositionX, go_transform);
		state.set_function("PositionY", &Transform::GetPositionY, go_transform);
		state.set_function("PositionZ", &Transform::GetPositionZ, go_transform);

		state.set_function("ScaleX", &Transform::GetScaleX, go_transform);
		state.set_function("ScaleY", &Transform::GetScaleY, go_transform);
		state.set_function("ScaleZ", &Transform::GetScaleZ, go_transform);

		state.set_function("RotateOrientation", &Transform::RotateOrientation, go_transform);
	}

	//Rigidbody functions
	RigidBody* go_rigidbody = p_game_obj->HasComponent<RigidBody>();
	if (go_rigidbody) {
		state.set_function("EnableRigidBody", &RigidBody::Enable, go_rigidbody);
	}
	
	state["has_collided"] = false;
}

void ScriptingManager::RegisterGUIFunctions(sol::state& state, GUIObject* p_gui_object) const {
	state.set_function("SetGUIScaleX", &GUIObject::SetScaleX, p_gui_object);
	state.set_function("SetGUIScaleY", &GUIObject::SetScaleY, p_gui_object);
	state.set_function("GetGUIScaleX", &GUIObject::GetScaleX, p_gui_object);
	state.set_function("GetGUIScaleY", &GUIObject::GetScaleY, p_gui_object);

	state.set_function("ChangeTexture", &GUIObject::ChangeTexture, p_gui_object);
	state.set_function("ChangeAltTexture", &GUIObject::ChangeAltTexture, p_gui_object);

	state.set_function("GetHighlighted", &GUIObject::GetHighlightState, p_gui_object);
	state.set_function("SetHighlighted", &GUIObject::SetHighlight, p_gui_object);

	state.set_function("DisableGUI", &GUIObject::Disable, p_gui_object);
	state.set_function("EnableGUI", &GUIObject::Enable, p_gui_object);

	state.set_function("DisableRender", &GUIObject::DisableRender, p_gui_object);
	state.set_function("EnableRender", &GUIObject::EnableRender, p_gui_object);

	GUIText* p_text = dynamic_cast<GUIText*>(p_gui_object);
	if (p_text != nullptr)
	{
		state.set_function("SetText", &GUIText::SetText, p_text);
	}

	Button* p_button = dynamic_cast<Button*>(p_gui_object);
	if (p_button != nullptr)
	{
		state.set_function("PlayClickSound", &Button::PlayClickSound, p_button);
		state.set_function("PlayDisabledClickSound", &Button::PlayDisabledClickSound, p_button);
	}

	RegisterActionsEnum(state);
	RegisterActionMap(state, &(p_gui_object_manager->gui_action_map));
}

void ScriptingManager::RegisterCameraFunctions(sol::state& state, RenderCam* p_cam_obj) const {
	state.set_function("SetCameraAspect",	&RenderCam::SetAspect, p_cam_obj);
	state.set_function("SetCameraPositon",	&RenderCam::SetPosition, p_cam_obj);
	state.set_function("SetCameraCenter",	&RenderCam::SetCameraCenter, p_cam_obj);
}

void ScriptingManager::RegisterActionMap(sol::state& state, InputAction* input_action) const {
	state.set_function("IsActionTriggered", &InputAction::IsActionTriggered, input_action);
	state.set_function("IsActionPressed", &InputAction::IsActionPressed, input_action);
	state.set_function("IsActionReleased", &InputAction::IsActionReleased, input_action);
	
	RegisterActionsEnum(state);
}

Vector<Behaviour*>& ScriptingManager::GetBehaviorListRef() {
	return behaviours_list;
}

void ScriptingManager::RegisterActionsEnum(sol::state& state) const {
	state.set("ACTION_UP",		InputAction::Actions::UP);
	state.set("ACTION_DOWN",	InputAction::Actions::DOWN);
	state.set("ACTION_LEFT",	InputAction::Actions::LEFT);
	state.set("ACTION_RIGHT",	InputAction::Actions::RIGHT);
	state.set("ACTION_1",		InputAction::Actions::ACTION_1);
	state.set("ACTION_2",		InputAction::Actions::ACTION_2);
	state.set("ACTION_3",		InputAction::Actions::ACTION_3);
	state.set("ACTION_4",		InputAction::Actions::ACTION_4);
	state.set("ACTION_L1",		InputAction::Actions::ACTION_L1);
	state.set("ACTION_L2",		InputAction::Actions::ACTION_L2);
	state.set("ACTION_R1",		InputAction::Actions::ACTION_R1);
	state.set("ACTION_R2",		InputAction::Actions::ACTION_R2);
	state.set("ACTION_START",	InputAction::Actions::ACTION_START);
	state.set("ACTION_SELECT",	InputAction::Actions::ACTION_SELECT);
	state.set("UP_ALT",			InputAction::Actions::UP_ALT);
	state.set("DOWN_ALT",		InputAction::Actions::DOWN_ALT);
	state.set("LEFT_ALT",		InputAction::Actions::LEFT_ALT);
	state.set("RIGHT_ALT",		InputAction::Actions::RIGHT_ALT);
}