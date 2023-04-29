#include "stdafx.h"
#include "PlayState.h"
#include "InputManager.h"
#include "GameManager.h"
#include "GameStateManager.h"
#include "Factory.h"
#include "MenuState.h"

PlayState::PlayState() {
}

PlayState::PlayState(String json_to_load) {
	printf(json_to_load.c_str());
}

void PlayState::Enter() {
}

void PlayState::Exit() {
}

void PlayState::Update() {
	//if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_L))
	//{
	//	p_gamestate_manager->TransitionToState(new MenuState("Menu State"));
	//}
}
