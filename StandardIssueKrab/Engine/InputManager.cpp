#include "stdafx.h"

#include "InputManager.h"

#include <imgui_impl_sdl.h>
#include "GameManager.h"
#include "GameStateManager.h"
#include "AudioManager.h"

// Default constructor
InputManager::InputManager() :
	keyboard_state_buffers{},
	current_keyboard_index{ keyboard_state_buffers.PushBack() },
	mouse_buffers{},
	current_mouse_index{ mouse_buffers.PushBack() } {
}

InputManager::~InputManager() {
	for (auto it = available_controllers.begin(); it != available_controllers.end(); ++it) {
		Controller* controller = it->second.get();
		SIK_INFO("Controller disconnected: controller_id({})", controller->controller_id);
		SDL_GameControllerClose(controller->connected_controller);
	}
}

void InputManager::InitializeController(SDL_ControllerDeviceEvent& event) {
	Int32 device_id = event.which;

	if (SDL_IsGameController(device_id)) {
		auto c = std::make_unique<Controller>();
		// The device_id used for SDL_GameControllerOpen() is NOT the
		// one used to identify the controller in future events (SDL Doc)
		c->connected_controller = SDL_GameControllerOpen(device_id);

		// Need to use SDL_JoystickInstanceID() to get correct id, see above
		SDL_Joystick* p_joystick = SDL_GameControllerGetJoystick(c->connected_controller);
		device_id = static_cast<Int32>(SDL_JoystickInstanceID(p_joystick));

		if (c->connected_controller) {
			c->controller_id = device_id;
			c->controller_data.controller_axis_buffers.PushBack();
			c->controller_data.current_controller_index = c->controller_data.controller_state_buffers.PushBack();

			Int32 map_id = GetNextFreeControllerId();
			SIK_INFO("Controller connected: map_id({}), controller_id({})", map_id, device_id);
			available_controllers[map_id] = std::move(c);
		}
		else {
			SIK_ERROR("Error opening controller: device_id({}): {}", device_id, SDL_GetError());
		}
	}

	RumbleCheck();
}

void InputManager::DisconnectController(SDL_ControllerDeviceEvent& event) {
	Int32 device_id = event.which;

	for (auto it = available_controllers.begin(); it != available_controllers.end(); ++it) {
		Controller* c = it->second.get();
		if (c->controller_id == device_id) {
			SIK_INFO("Controller disconnected: controller_id({})", c->controller_id);
			SDL_GameControllerClose(c->connected_controller);
			available_controllers.erase(it);
			
			// TODO - handle player one controller disconnect SIK-158

			break;
		}
	}
}

void InputManager::Update() {
	
	SDL_Event e{};
	while (SDL_PollEvent(&e) != 0) {
	
		switch (e.type) {
		// Checking for quit event
		case SDL_QUIT:
			p_game_manager->Quit();
			break;
		// Checking if controller is connected
		case SDL_CONTROLLERDEVICEADDED:
			p_input_manager->InitializeController(e.cdevice);
			ShowMouseCursor(false);
			break;
		// Checking if controller is removed
		case SDL_CONTROLLERDEVICEREMOVED:
			p_input_manager->DisconnectController(e.cdevice);
			ShowMouseCursor(true);
			break;
		case SDL_WINDOWEVENT:
		{
			switch (e.window.event)
			{
			case SDL_WINDOWEVENT_MINIMIZED:
			{
				p_audio_manager->MuteAll();
				break;
			}
			case SDL_WINDOWEVENT_RESTORED:
			{
				p_audio_manager->UnMuteAll();
				break;
			}
			}
			break;
		}
		default:
			break;
		}

		// For imgui window to handle io events
		ImGui_ImplSDL2_ProcessEvent(&e);

		// Allow game states to handle raw events
		p_gamestate_manager->HandleEvent(e);
	}

	//Don't process inputs if imgui has control of the mouse
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	// Cache keyboard states every frame
	int numberOfItems = 0;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(&numberOfItems);

	if (numberOfItems > 512) {
		numberOfItems = 512;
	}

	current_keyboard_index = keyboard_state_buffers.PushBack();
	memcpy(keyboard_state_buffers.At(current_keyboard_index).data(), currentKeyStates, numberOfItems);

	// Cache mouse states and cursor position every frame
	Mouse prev_mouse = mouse_buffers.At(current_mouse_index);
	current_mouse_index = mouse_buffers.PushBack();
	mouse_buffers.At(current_mouse_index).mouse_state = SDL_GetMouseState(&mouse_buffers.At(current_mouse_index).mouse_pos.x,
		&mouse_buffers.At(current_mouse_index).mouse_pos.y);
	Mouse curr_mouse = mouse_buffers.At(current_mouse_index);
	
	mouse_moved_rolling_avg *= 0.98f;
	if (glm::abs(curr_mouse.mouse_pos.x - prev_mouse.mouse_pos.x) > mouse_moved_threshold ||
		glm::abs(curr_mouse.mouse_pos.y - prev_mouse.mouse_pos.y) > mouse_moved_threshold) {
		mouse_moved_rolling_avg += 0.02f;
		if (not IsMouseCursorShown()) {
			ShowMouseCursor(true);
		}
	}
	if (mouse_moved_rolling_avg < 0.0001f) {
		if (IsMouseCursorShown()) {
			ShowMouseCursor(false);
		}
	}
	

	// Controller axis and button update
	for (auto it = available_controllers.begin(); it != available_controllers.end(); ++it) {
		Controller* c = it->second.get();
		SIK_ASSERT(c != nullptr && c->connected_controller != nullptr,
			"Invalid controller");
		if (c != nullptr && c->connected_controller != nullptr) {
			c->controller_data.controller_axis_buffers.PushBack();
			c->controller_data.current_controller_index = c->controller_data.controller_state_buffers.PushBack();

			// Axes
			for (Uint8 i = 0; i < 6; ++i) {
				c->controller_data.controller_axis_buffers.At(c->controller_data.current_controller_index).at(i)
					= SDL_GameControllerGetAxis(c->connected_controller, static_cast<SDL_GameControllerAxis>(i));
			}
			// Buttons
			for (Uint8 i = 0; i < 21; ++i) {
				c->controller_data.controller_state_buffers.At(c->controller_data.current_controller_index).at(i)
					= SDL_GameControllerGetButton(c->connected_controller, static_cast<SDL_GameControllerButton>(i));
			}
		}
	}
}

// Checking if the Key is pressed and returning true of false accordingly
Bool InputManager::IsKeyPressed(Uint32 key_down_value) const {
	if (key_down_value >= 512)
		return false;

	return keyboard_state_buffers.At(current_keyboard_index).at(key_down_value);
}

// Checking if the Key is released and returning true of false accordingly
Bool InputManager::IsKeyReleased(Uint32 key_down_value) const {
	if (key_down_value >= 512)
		return false;

	return keyboard_state_buffers.At(current_keyboard_index - 1).at(key_down_value) != 0 &&
		keyboard_state_buffers.At(current_keyboard_index).at(key_down_value) == 0;
}

// Checking if the Key is triggered and returning true of false accordingly
Bool InputManager::IsKeyTriggered(Uint32 key_down_value) const {
	if (key_down_value >= 512)
		return false;

	return keyboard_state_buffers.At(current_keyboard_index - 1).at(key_down_value) == 0 &&
		keyboard_state_buffers.At(current_keyboard_index).at(key_down_value) != 0;
}

// Returns current position of mouse
Ivec2 InputManager::GetMousePos() const {
	return mouse_buffers.At(current_mouse_index).mouse_pos;
}

// Returns difference vector of mouse pos
Ivec2 InputManager::GetMouseDelta() const {
	// curr mouse pos - prev mouse pos
	return mouse_buffers.At(current_mouse_index).mouse_pos - mouse_buffers.At(current_mouse_index - 1).mouse_pos;
}

// Checks if any mouse button is pressed
Bool InputManager::IsMouseButtonPressed(Uint32 mouse_button_index) const {
	return static_cast<Bool>(mouse_buffers.At(current_mouse_index).mouse_state & mouse_button_index);
}

// Checks if any mouse button is released
Bool InputManager::IsMouseButtonReleased(Uint32 mouse_button_index) const {
	return static_cast<Bool>(mouse_buffers.At(current_mouse_index - 1).mouse_state & mouse_button_index) &&
		!static_cast<Bool>(mouse_buffers.At(current_mouse_index).mouse_state & mouse_button_index);
}

// Checks if any mouse button is triggered
Bool InputManager::IsMouseButtonTriggered(Uint32 mouse_button_index) const {
	return !static_cast<Bool>(mouse_buffers.At(current_mouse_index - 1).mouse_state & mouse_button_index) &&
		static_cast<Bool>(mouse_buffers.At(current_mouse_index).mouse_state & mouse_button_index);
}

// Check if controller is available
Bool InputManager::IsControllerAvailable(Uint32 controller_id) const {
	return available_controllers.count(controller_id) > 0;
}

// Checks if any controller axis is pressed positive
Bool InputManager::IsAxisPressedPositive(Uint32 controller_id, Int8 axis_scan_code) const {
	if (axis_scan_code >= 6 || axis_scan_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return (it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index)[axis_scan_code] > JOYSTICK_DEADZONE);
	}
	return false;
}

// Checks if any controller axis is pressed negative
Bool InputManager::IsAxisPressedNegative(Uint32 controller_id, Int8 axis_scan_code) const {
	if (axis_scan_code >= 6 || axis_scan_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return (it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index)[axis_scan_code] < -JOYSTICK_DEADZONE);
	}
	return false;
}

// Checks if any controller axis is triggered positive
Bool InputManager::IsAxisTriggeredPositive(Uint32 controller_id, Int8 axis_scan_code) const {
	if (axis_scan_code >= 6 || axis_scan_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return (it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index)[axis_scan_code] > JOYSTICK_DEADZONE) &&
			(it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index - 1)[axis_scan_code] < JOYSTICK_DEADZONE);
	}
	return false;
}

// Checks if any controller axis is triggered negative
Bool InputManager::IsAxisTriggeredNegative(Uint32 controller_id, Int8 axis_scan_code) const {
	if (axis_scan_code >= 6 || axis_scan_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return (it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index)[axis_scan_code] < -JOYSTICK_DEADZONE) &&
			(it->second->controller_data.controller_axis_buffers.At(it->second->controller_data.current_controller_index - 1)[axis_scan_code] > -JOYSTICK_DEADZONE);
	}
	return false;
}

// Checks if any controller button is pressed
Bool InputManager::IsControllerButtonPressed(Uint32 controller_id, Int8 controller_button_down_code) const {
	if (controller_button_down_code >= 21 || controller_button_down_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return it->second->controller_data.controller_state_buffers.At(it->second->controller_data.current_controller_index)[controller_button_down_code];
	}
	return false;
}

// Checks if any controller button is released
Bool InputManager::IsControllerButtonReleased(Uint32 controller_id, Int8 controller_button_down_code) const {
	if (controller_button_down_code >= 21 || controller_button_down_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return !(it->second->controller_data.controller_state_buffers.At(it->second->controller_data.current_controller_index)[controller_button_down_code]) &&
			(it->second->controller_data.controller_state_buffers.At(it->second->controller_data.current_controller_index - 1)[controller_button_down_code]);
	}
	return false;
}

// Checks if any controller button is triggered
Bool InputManager::IsControllerButtonTriggered(Uint32 controller_id, Int8 controller_button_down_code) const {
	if (controller_button_down_code >= 21 || controller_button_down_code < 0)
		return false;

	auto it = available_controllers.find(controller_id);
	if (it != available_controllers.end()) {
		return (it->second->controller_data.controller_state_buffers.At(it->second->controller_data.current_controller_index)[controller_button_down_code]) &&
			!(it->second->controller_data.controller_state_buffers.At(it->second->controller_data.current_controller_index - 1)[controller_button_down_code]);
	}
	return false;
}

// Check to see if the connected controller supports rumble or not
void InputManager::RumbleCheck() {

	for (int i = 0; i < available_controllers.size(); ++i) {

		if (!SDL_GameControllerHasRumble(available_controllers[i]->connected_controller)) {
			SIK_ERROR("The controller does not support rumble");
		}
		else {
			SIK_INFO("Controller Supports rumble");
		}
	}
}


void InputManager::RumbleController(Uint16 low_frequency, Uint16 high_frequency, Uint32 duration_ms) {

	if (available_controllers.size() == 0) {
		return;
	}

	int value = SDL_GameControllerRumble(available_controllers[0]->connected_controller, low_frequency, high_frequency, duration_ms);
}

// Toggle to show/unshow mouse cursor
void InputManager::ShowMouseCursor(Bool val) {
	cursor_toggle = val;
	SDL_ShowCursor(val);
}
Bool InputManager::ToggleMouseCursor() {
	cursor_toggle = !cursor_toggle;
	SDL_ShowCursor(cursor_toggle);
	return cursor_toggle;
}
Bool InputManager::IsMouseCursorShown() const {
	return cursor_toggle;
}

Bool InputManager::AreControllersConnected() {
	return (not available_controllers.empty());
}

/*
* Gets the controller ID for the player 1 controller
* Need a way to figure out which controller is the primary one
* Returns: Int8 - The controller id for player 1
*/
Int8 InputManager::GetPlayerOneController() {
	Int8 ret = -1;
	if (not available_controllers.empty()) {
		auto it = available_controllers.begin();
		ret = static_cast<Int8>(it->first);
	}
	return ret;
}

Int8 InputManager::GetPlayerController(Uint8 player_number) {
	Int8 ret = -1;
	auto iter = available_controllers.begin();
	Uint8 iter_count = player_number;
	while (iter != available_controllers.end()) {
		if (iter_count == 1) {
			ret = static_cast<Int8>(iter->first);
			break;
		}
		iter_count--;
		iter++;
	}
	return ret;
}

Int32 InputManager::GetNextFreeControllerId() {
	Int32 id = -1;
	for (Int32 i = 0; i < SDL_NumJoysticks(); ++i) {
		if (available_controllers.count(i) == 0) {
			id = i;
			break;
		}
	}
	return id;
}
