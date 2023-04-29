#include "stdafx.h"
#include "StartState.h"
#include "FadeState.h"

#include "Engine/GUIObject.h"
#include "Engine/FadePanel.h"
#include "Engine/Button.h"
#include "Engine/MemoryManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GameStateManager.h"
#include "Engine/GameManager.h"
#include "Engine/AudioManager.h"
#include "Engine/ResourceManager.h"
#include "Engine/InputManager.h"
#include "Engine/Behaviour.h"

#include "ScriptInterface.h"
#include "MenuState.h"

void StartAction() {
	p_gamestate_manager->PopState();
}

void QuitAction() {
	p_gamestate_manager->PopState();
	p_game_manager->Quit();
}

StartState::StartState(const char* menu_json) 
	: menu_objects{ p_gui_object_manager->CreateGUIFromFile(menu_json)}, 
	splash_screen_objects{ p_gui_object_manager->CreateGUIFromFile("splash_screen.json") },
	credits_objects{ p_gui_object_manager->CreateGUIFromFile("credits_screen.json") },
	options_objects{ p_gui_object_manager->CreateGUIFromFile("options_menu.json") },
	quit_confirmation_objects{ p_gui_object_manager->CreateGUIFromFile("quit_confirmation.json") },
	menu_back_sound{ -1 },
	timer{ 2.0f },
	screen_idx{ static_cast<Int32>(splash_screen_objects.size()) - 1 },
	curr_state{State::Splash},
	first_time{ true }
{
	// Move DigiPen logo to back and SIK to front
	for (auto&& obj : splash_screen_objects) {
		if (obj->GetName() == "DigiPen"_sid) {
			std::swap(obj, splash_screen_objects.back());
			break;
		}
	}
	for (auto&& obj : splash_screen_objects) {
		if (obj->GetName() == "SIK"_sid) {
			std::swap(obj, splash_screen_objects.front());
			break;
		}
	}

	// Sort the credits objects
	for (auto&& obj : credits_objects) {
		if (obj->GetName() == "Credits1"_sid) {
			std::swap(obj, credits_objects.back());
			break;
		}
	}
	for (auto&& obj : credits_objects) {
		if (obj->GetName() == "Credits4"_sid) {
			std::swap(obj, credits_objects.front());
			break;
		}
	}
	if (auto& obj = credits_objects[1];  obj->GetName() == "Credits2"_sid) {
		std::swap(obj, credits_objects[2]);
	}

	for (auto&& obj : options_objects) {
		obj->Disable();
	}

	for (auto& gui_object : menu_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaStartState(script.script_state,
					this);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {

					RegisterLuaStartState(script.script_state,
						this);
				}
			}
		}
	}

	for (auto& gui_object : options_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaStartState(script.script_state,
					this);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {

					RegisterLuaStartState(script.script_state,
						this);
				}
			}
		}
	}
	for (auto& gui_object : quit_confirmation_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaStartState(script.script_state,
					this);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {

					RegisterLuaStartState(script.script_state,
						this);
				}
			}
		}
	}
}

StartState::~StartState() {
}

void StartState::Enter() {	
	if (first_time) { 
		SplashScreens(); 
		first_time = false; 
	}

	for (auto& obj : menu_objects) {
		obj->Enable();
	}

	menu_back_sound = p_audio_manager->PlayAudio("MENU_BACK"_sid,
		p_audio_manager->background_music_chanel_group,
		10.0,
		1.0f,
		false,
		-1);
}

void StartState::Exit() {
	for (auto& obj : menu_objects) {
		obj->Disable();
	}
	
	for (auto& obj : splash_screen_objects) {
		obj->Disable();
	}

	for (auto& obj : credits_objects) {
		obj->Disable();
	}

	for (auto&& obj : options_objects) {
		obj->Disable();
	}

	
	for (auto& obj : quit_confirmation_objects) {
		obj->Disable();
	}

	p_audio_manager->Stop(menu_back_sound);

	p_gui_object_manager->RemoveAllHighlightObjects();
}

void StartState::RollCredits() {
	curr_state = State::Credits;
	timer = 2.0f;
	screen_idx = static_cast<Int32>(credits_objects.size()) - 1;
	p_gui_object_manager->RemoveAllHighlightObjects();
	for (auto&& obj : splash_screen_objects) {
		obj->Disable();
	}
	for (auto&& obj : menu_objects) {
		obj->Disable();
	}
	for (auto&& obj : options_objects) {
		obj->Disable();
	}
	for (auto&& obj : quit_confirmation_objects) {
		obj->Disable();
	}
	for (auto&& obj : credits_objects) {
		obj->Enable();
	}
}

void StartState::SplashScreens() {
	curr_state = State::Splash;
	timer = 2.0f;
	screen_idx = static_cast<Int32>(splash_screen_objects.size()) - 1;
	p_gui_object_manager->RemoveAllHighlightObjects();
	for (auto&& obj : credits_objects) {
		obj->Disable();
	}
	for (auto&& obj : menu_objects) {
		obj->Disable();
	}
	for (auto&& obj : options_objects) {
		obj->Disable();
	}
	for (auto&& obj : quit_confirmation_objects) {
		obj->Disable();
	}
	for (auto&& obj : splash_screen_objects) {
		obj->Enable();
	}
}

void StartState::OptionsMenu() {
	curr_state = State::Options;
	p_gui_object_manager->RemoveAllHighlightObjects();
	for (auto&& obj : menu_objects) {
		obj->Disable();
	}
	for (auto&& obj : splash_screen_objects) {
		obj->Disable();
	}
	for (auto&& obj : credits_objects) {
		obj->Disable();
	}
	for (auto&& obj : quit_confirmation_objects) {
		obj->Disable();
	}
	for (auto&& obj : options_objects) {
		obj->Enable();
	}
}

void StartState::ConfirmQuit() {
	curr_state = State::QuitConfirmation;
	p_gui_object_manager->RemoveAllHighlightObjects();
	for (auto&& obj : splash_screen_objects) {
		obj->Disable();
	}
	for (auto&& obj : credits_objects) {
		obj->Disable();
	}
	for (auto&& obj : options_objects) {
		obj->Disable();
	}
	/*for (auto&& obj : menu_objects) {
		obj->Disable();
	}*/
	for (auto&& obj : quit_confirmation_objects) {
		obj->Enable();
	}
}

void StartState::MainMenu() {
	curr_state = State::Menu;
	p_gui_object_manager->RemoveAllHighlightObjects();
	for (auto&& obj : splash_screen_objects) {
		obj->Disable();
	}
	for (auto&& obj : credits_objects) {
		obj->Disable();
	}
	for (auto&& obj : quit_confirmation_objects) {
		obj->Disable();
	}
	for (auto&& obj : menu_objects) {
		obj->Disable();
	}
	for (auto&& obj : splash_screen_objects) {
		obj->Disable();
	}
	for (auto&& obj : credits_objects) {
		obj->Disable();
	}
	for (auto&& obj : options_objects) {
		obj->Disable();
	}
	for (auto&& obj : menu_objects) {
		obj->Enable();
	}
}

void StartState::PushStartState() {
	p_gamestate_manager->PushState(p_start_state);
}

void StartState::Update(Float32 dt) {
	
	switch (curr_state)
	{
		break; case State::Menu: {
			p_gui_object_manager->UpdateUIControls(dt);
			for (auto& obj : menu_objects) {
				obj->Update(dt);
			}
		}
		break; case State::Options: {
			p_gui_object_manager->UpdateUIControls(dt);
			for (auto& obj : options_objects) {
				obj->Update(dt);
			}
		}
		break; case State::QuitConfirmation: {
			p_gui_object_manager->UpdateUIControls(dt);
			for (auto& obj : quit_confirmation_objects) {
				obj->Update(dt);
			}
		}
		break; case State::Splash: {
			if (screen_idx >= 0) {
				timer -= dt;
				if (timer <= 0.0f) {
					// Disable the top splash screen and reset the timer
					splash_screen_objects[screen_idx--]->Disable();
					timer = 2.0f;
					if (screen_idx < 0) {
						MainMenu();
					}
				}
			}
		}
		break; case State::Credits: {
			if (screen_idx >= 0) {
				timer -= dt;
				if (timer <= 0.0f) {
					// Disable the top credits screen and reset the timer
					credits_objects[screen_idx--]->Disable();
					timer = 2.0f;
					if (screen_idx < 0) {
						MainMenu();
					}
				}
			}
		}
	}
}
