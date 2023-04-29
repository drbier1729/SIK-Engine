#pragma once

typedef enum
{
    SDL_MOUSE_BUTTON_INVALID = 0,
	SDL_MOUSE_BUTTON_LEFT = SDL_BUTTON_LMASK,
	SDL_MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MMASK,
	SDL_MOUSE_BUTTON_RIGHT = SDL_BUTTON_RMASK,
	SDL_MOUSE_BUTTON_X1 = SDL_BUTTON_X1MASK,
	SDL_MOUSE_BUTTON_X2 = SDL_BUTTON_X2MASK
} SDL_MouseButton;

class InputAction {
public:

	/*
	* All the possible action supported by the engine.
	* More actions can be added BUT you !!MUST!! add the corresponding string entry
	* in string_action_map in InputAction.cpp 
	*/
	enum class Actions {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		ACTION_1,
		ACTION_2,
		ACTION_3,
		ACTION_4,
		ACTION_L1,
		ACTION_L2,
		ACTION_R1,
		ACTION_R2,
		ACTION_START,
		ACTION_SELECT,
		UP_ALT,
		DOWN_ALT,
		LEFT_ALT,
		RIGHT_ALT,
		_NUM
	};


	InputAction() = delete;

	/*
	* Create an action map from a specified config file
	*/
	InputAction(const char* config_file_path, Uint8 player_gamepad=1);

	~InputAction();

	/*
	* Checks if a particular action was triggered on the current frame
	* Returns: bool - True if triggered
	*/
	bool IsActionTriggered(Actions action);

	/*
	* Checks if a particular action is pressed on the current frame
	* Returns: bool - True if triggered
	*/
	bool IsActionPressed(Actions action);

	/*
	* Checks if a particular action was pressed on the previous frame
	* but released on the current frame
	* Returns: bool - True if triggered
	*/
	bool IsActionReleased(Actions action);

	/*
	* Sets the player controller number which determines which
	* gamepad to use.
	* Returns : void
	*/
	void SetPlayerControllerNum(Uint8 _player_controller_num);
private:
	UnorderedMap<Actions, SDL_Scancode> keyboard_maps;
	//Axis maps have to map an action to a particular Axis as well as direction
	UnorderedMap<Actions, std::pair<SDL_GameControllerAxis, Int8>> axis_maps;
	UnorderedMap<Actions, SDL_GameControllerButton> controller_maps;
	UnorderedMap<Actions, SDL_MouseButton> mouse_maps;

	Uint8 player_controller_num;
	/*
	* Creates the Maps based on the config_file_path
	* Returns:void
	*/
	void LoadMap(const char* config_file_path);

	static std::unordered_map<StringID, Actions> string_action_map;
};

