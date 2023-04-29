#pragma once

#include "RingBuffer.h"

constexpr Int16 JOYSTICK_DEADZONE = 8000;

class InputManager
{
public:
	InputManager();
	~InputManager();

	void InitializeController(SDL_ControllerDeviceEvent& event);
	void DisconnectController(SDL_ControllerDeviceEvent& event);

	void Update();

	Bool IsKeyPressed(Uint32 key_down_value) const;
	Bool IsKeyReleased(Uint32 key_down_value) const;
	Bool IsKeyTriggered(Uint32 key_down_value) const;

	Ivec2 GetMousePos() const;
	Ivec2 GetMouseDelta() const;

	Bool IsMouseButtonPressed(Uint32 mouse_button_index) const;
	Bool IsMouseButtonReleased(Uint32 mouse_button_index) const;
	Bool IsMouseButtonTriggered(Uint32 mouse_button_index) const;

	Bool IsControllerAvailable(Uint32 controller_id) const;

	Bool IsAxisPressedPositive(Uint32 controller_id, Int8 axis_scan_code) const;
	Bool IsAxisPressedNegative(Uint32 controller_id, Int8 axis_scan_code) const;
	Bool IsAxisTriggeredPositive(Uint32 controller_id, Int8 axis_scan_code) const;
	Bool IsAxisTriggeredNegative(Uint32 controller_id, Int8 axis_scan_code) const;
		 
	Bool IsControllerButtonPressed(Uint32 controller_id, Int8 controller_button_down_code) const;
	Bool IsControllerButtonReleased(Uint32 controller_id, Int8 controller_button_down_code) const;
	Bool IsControllerButtonTriggered(Uint32 controller_id, Int8 controller_button_down_code) const;

	void RumbleCheck();
	void RumbleController(Uint16 low_frequency, Uint16 high_frequency, Uint32 duration_ms);

	void ShowMouseCursor(Bool val = true);
	Bool ToggleMouseCursor();
	Bool IsMouseCursorShown() const;

	Bool AreControllersConnected();
	/*
	* Gets the controller ID for the player 1 controller
	* Need a way to figure out which controller is the primary one
	* Returns: Int8 - The controller id for player 1
	*/
	Int8 GetPlayerOneController();

	/*
	* Get the controller ID for the specified player controller
	* Returns: Int8 - the controller id for the spcified player
	*/
	Int8 GetPlayerController(Uint8 player_number);
private:
	// Keyboard
	RingBuffer<Array<Uint8, 512>, 32> keyboard_state_buffers;
	SizeT current_keyboard_index;

	// Mouse
	struct Mouse {
		Uint32 mouse_state;
		Ivec2 mouse_pos;
	};
	RingBuffer<Mouse, 32> mouse_buffers;
	SizeT current_mouse_index;

	// Controller
	struct ControllerData {
		RingBuffer<Array<Int8, 21>, 32> controller_state_buffers;
		RingBuffer<Array<Int16, 6>, 6> controller_axis_buffers;
		SizeT current_controller_index;
	};
	struct Controller {
		SDL_GameController* connected_controller = nullptr;
		Int32 controller_id = -1;
		ControllerData controller_data = {};
	};

	UnorderedMap<Int32, UniquePtr<Controller>> available_controllers;
	Int32 GetNextFreeControllerId();

	Bool cursor_toggle = true;
	Float32 mouse_moved_rolling_avg = 0.0f;
	Int32 mouse_moved_threshold = 3;
};

//Declared as an extern variable so it can be accessed throughout the project
extern InputManager* p_input_manager;