#include "stdafx.h"
#include <tuple>
#include "InputManager.h"
#include "ResourceManager.h"
#include "InputAction.h"


std::unordered_map<StringID, InputAction::Actions> InputAction::string_action_map{
	{"UP"_sid, InputAction::Actions::UP},
	{"DOWN"_sid, InputAction::Actions::DOWN},
	{"LEFT"_sid, InputAction::Actions::LEFT},
	{"RIGHT"_sid, InputAction::Actions::RIGHT},
	{"ACTION_1"_sid, InputAction::Actions::ACTION_1},
	{"ACTION_2"_sid, InputAction::Actions::ACTION_2},
	{"ACTION_3"_sid, InputAction::Actions::ACTION_3},
	{"ACTION_4"_sid, InputAction::Actions::ACTION_4},
	{"ACTION_L1"_sid, InputAction::Actions::ACTION_L1},
	{"ACTION_L2"_sid, InputAction::Actions::ACTION_L2},
	{"ACTION_R1"_sid, InputAction::Actions::ACTION_R1},
	{"ACTION_R2"_sid, InputAction::Actions::ACTION_R2},
	{"ACTION_START"_sid, InputAction::Actions::ACTION_START},
	{"ACTION_SELECT"_sid, InputAction::Actions::ACTION_SELECT},
	{"UP_ALT"_sid, InputAction::Actions::UP_ALT},
	{"DOWN_ALT"_sid, InputAction::Actions::DOWN_ALT},
	{"LEFT_ALT"_sid, InputAction::Actions::LEFT_ALT},
	{"RIGHT_ALT"_sid, InputAction::Actions::RIGHT_ALT}
};

/*
* Create an action map from a specified config file
*/
InputAction::InputAction(const char* config_file_path, Uint8 player_gamepad) : 
	player_controller_num(player_gamepad) {
	LoadMap(config_file_path);
}

/*
* Clear all the maps
*/
InputAction::~InputAction() {
	keyboard_maps.clear();
	mouse_maps.clear();
	controller_maps.clear();
	axis_maps.clear();
}

/*
* Checks if a particular action was triggered on the current frame
* Returns: bool - True if triggered
*/
bool InputAction::IsActionTriggered(Actions action) {
	if (p_input_manager->IsKeyTriggered(keyboard_maps[action]) ||
		p_input_manager->IsMouseButtonTriggered(mouse_maps[action]) ||
		p_input_manager->IsControllerButtonTriggered(
			p_input_manager->GetPlayerController(player_controller_num),
			static_cast<Int8>(controller_maps[action]))
		)
		return true;
	
	//Check for axis has to be done separately since there is a positive and negative axis
	SDL_GameControllerAxis axis_code = axis_maps[action].first;
	Int8 direction = axis_maps[action].second;
	if (direction > 0) {
		if (p_input_manager->IsAxisTriggeredPositive(
			p_input_manager->GetPlayerController(player_controller_num),
			static_cast<Int8>(axis_code)))
			return true;
	}
	else if (direction < 0) {
		if (p_input_manager->IsAxisTriggeredNegative(
			p_input_manager->GetPlayerController(player_controller_num),
			static_cast<Int8>(axis_code)))
			return true;
	}

	return false;
}

/*
* Checks if a particular action is pressed on the current frame
* Returns: bool - True if triggered
*/
bool InputAction::IsActionPressed(Actions action) {
	if (p_input_manager->IsKeyPressed(keyboard_maps[action]) ||
		p_input_manager->IsMouseButtonPressed(mouse_maps[action]) ||
		p_input_manager->IsControllerButtonPressed(
			p_input_manager->GetPlayerController(player_controller_num), 
			static_cast<Int8>(controller_maps[action])))
		return true;

	//Check for axis has to be done separately since there is a positive and negative axis
	SDL_GameControllerAxis axis_code = axis_maps[action].first;
	Int8 direction = axis_maps[action].second;
	if (direction > 0) {
		if (p_input_manager->IsAxisPressedPositive(
			p_input_manager->GetPlayerController(player_controller_num), 
			static_cast<Int8>(axis_code)))
			return true;
	}
	else if (direction < 0) {
		if (p_input_manager->IsAxisPressedNegative(
			p_input_manager->GetPlayerController(player_controller_num), 
			static_cast<Int8>(axis_code)))
			return true;
	}

	return false;
}

/*
* Checks if a particular action was pressed on the previous frame
* but released on the current frame
* Returns: bool - True if triggered
*/
bool InputAction::IsActionReleased(Actions action) {
	if (p_input_manager->IsKeyReleased(keyboard_maps[action]) ||
		p_input_manager->IsMouseButtonReleased(mouse_maps[action]) ||
		p_input_manager->IsControllerButtonReleased(
			p_input_manager->GetPlayerController(player_controller_num), 
			static_cast<Int8>(controller_maps[action]))
		)
		return true;

	return false;
}

void InputAction::SetPlayerControllerNum(Uint8 _player_controller_num) {
	player_controller_num = _player_controller_num;
}
/*
* Creates the Maps based on the config_file_path
* Returns:void
*/
void InputAction::LoadMap(const char* config_file_name) {

	//Check if we want to load the default action map
	if (strcmp(config_file_name, "default") != 0) {
		JSON* doc{ p_resource_manager->LoadJSON(config_file_name) };
		rapidjson::Value::ConstMemberIterator doc_itr;

#define SET_ACTION_VALUE(action, set_action) \
        doc_itr = json_action.value.FindMember(action);\
        if (doc_itr != json_action.value.MemberEnd()) {\
            if (doc_itr->value.IsInt()) {\
                set_action;\
            }\
            else {\
                SIK_WARN("{} Action Code is not an Int", action); \
            }\
        }

		for (auto& json_action : doc->doc.GetObj()) {
			const char* action_string = json_action.name.GetString();
			auto itr = string_action_map.find(ToStringID(action_string));
			SIK_ASSERT(itr != string_action_map.end(), "Item not found in string action map");
			
			//Check for keyboard action
			SET_ACTION_VALUE("Keyboard",
				keyboard_maps.emplace(itr->second, SDL_Scancode(doc_itr->value.GetInt())));
			//Check for controller action
			SET_ACTION_VALUE("Controller",
				controller_maps.emplace(itr->second, SDL_GameControllerButton(doc_itr->value.GetInt())));
			//Check for mouse action
			SET_ACTION_VALUE("Mouse",
				mouse_maps.emplace(itr->second, SDL_MouseButton(doc_itr->value.GetInt())));

			//Check for Axis action
			//Is different from other actions since Axis actions contain an axis code and direction
			doc_itr = json_action.value.FindMember("Axis");
			if (doc_itr != json_action.value.MemberEnd()) {
				if (doc_itr->value.IsArray()) {
					auto axis_array = doc_itr->value.GetArray();
					int action_val = axis_array[0].GetInt();
					int direction = axis_array[1].GetInt();
					axis_maps.emplace(
						itr->second,
						std::make_pair(SDL_GameControllerAxis(action_val), direction)
					);
				}
				else {
					SIK_WARN("Axis Action is not an Array");
				}
			}
		}
#undef SET_ACTION_VALUE
		return;
	}

	//Iterate over all the actions to set the default actions
	for (Actions action = static_cast<Actions>(0) ;
		action < Actions::_NUM; 
		action = static_cast<Actions>((SizeT)action + 1)) {

		switch (action) {
		case Actions::UP:
			keyboard_maps.emplace(
				Actions::UP,
				SDL_SCANCODE_UP
			);
			
			controller_maps.emplace(
				Actions::UP,
				SDL_CONTROLLER_BUTTON_DPAD_UP
			);

			axis_maps.emplace(
				Actions::UP,
				//1 indicates a positive direction for the Leftstick in the Y-axis
				std::make_pair(SDL_CONTROLLER_AXIS_LEFTY, 1)
			);
			break;
		case Actions::DOWN:
			keyboard_maps.emplace(
				Actions::DOWN,
				SDL_SCANCODE_DOWN
			);

			controller_maps.emplace(
				Actions::DOWN,
				SDL_CONTROLLER_BUTTON_DPAD_DOWN
			);

			axis_maps.emplace(
				Actions::DOWN,
				//-1 indicates a negative direction for the Leftstick in the Y-axis
				std::make_pair(SDL_CONTROLLER_AXIS_LEFTY, -1)
			);
			break;
		case Actions::LEFT:
			keyboard_maps.emplace(
				Actions::LEFT,
				SDL_SCANCODE_LEFT
			);

			controller_maps.emplace(
				Actions::LEFT,
				SDL_CONTROLLER_BUTTON_DPAD_LEFT
			);

			axis_maps.emplace(
				Actions::LEFT,
				//-1 indicates a negative direction for the Leftstick in the X-axis
				std::make_pair(SDL_CONTROLLER_AXIS_LEFTX, -1)
			);
			break;
		case Actions::RIGHT:
			keyboard_maps.emplace(
				Actions::RIGHT,
				SDL_SCANCODE_RIGHT
			);

			controller_maps.emplace(
				Actions::RIGHT,
				SDL_CONTROLLER_BUTTON_DPAD_RIGHT
			);

			axis_maps.emplace(
				Actions::RIGHT,
				//1 indicates a positive direction for the Leftstick in the X-axis
				std::make_pair(SDL_CONTROLLER_AXIS_LEFTX, 1)
			);
			break;
		case Actions::ACTION_1:
			keyboard_maps.emplace(
				Actions::ACTION_1,
				SDL_SCANCODE_SPACE
			);

			controller_maps.emplace(
				Actions::ACTION_1,
				SDL_CONTROLLER_BUTTON_A
			);

			mouse_maps.emplace(
				Actions::ACTION_1,
				SDL_MOUSE_BUTTON_LEFT
			);
			break;
		case Actions::ACTION_2:
			keyboard_maps.emplace(
				Actions::ACTION_2,
				SDL_SCANCODE_RSHIFT
			);

			controller_maps.emplace(
				Actions::ACTION_2,
				SDL_CONTROLLER_BUTTON_B
			);

			mouse_maps.emplace(
				Actions::ACTION_2,
				SDL_MOUSE_BUTTON_RIGHT
			);
			break;
		case Actions::ACTION_3:
			break;
		case Actions::ACTION_4:
			break;
		case Actions::ACTION_L1:
			break;
		case Actions::ACTION_L2:
			break;
		case Actions::ACTION_R1:
			break;
		case Actions::ACTION_R2:
			break;
		case Actions::ACTION_START:
			keyboard_maps.emplace(
				Actions::ACTION_START,
				SDL_SCANCODE_RETURN
			);

			controller_maps.emplace(
				Actions::ACTION_START,
				SDL_CONTROLLER_BUTTON_START
			);
			break;
		case Actions::ACTION_SELECT:
			keyboard_maps.emplace(
				Actions::ACTION_SELECT,
				SDL_SCANCODE_BACKSPACE
			);

			controller_maps.emplace(
				Actions::ACTION_SELECT,
				SDL_CONTROLLER_BUTTON_BACK
			);
			break;
		case Actions::UP_ALT:
			break;
		case Actions::DOWN_ALT:
			break;
		case Actions::LEFT_ALT:
			break;
		case Actions::RIGHT_ALT:
			break;
		case Actions::_NUM:
			//"Default" case. Will never reach here
			break;
		}
	}
}

