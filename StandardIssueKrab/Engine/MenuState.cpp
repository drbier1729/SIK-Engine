#include "stdafx.h"
#include "MenuState.h"
#include "PlayState.h"
#include "InputManager.h"
#include "GraphicsManager.h"
#include "GameStateManager.h"
#include "AudioManager.h"

MenuState::MenuState() {
}

MenuState::MenuState(String json_to_load) {
	printf(json_to_load.c_str());
}

void MenuState::Enter() {
	
}

void MenuState::Exit() {
}

void MenuState::Update() {
	//if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_L))
	//{
	//	p_gamestate_manager->TransitionToState(new PlayState("Play State"));
	//}
}

MenuState::~MenuState() {
}
