#pragma once
#include "Engine\GameState.h"

class GUIObject;
class MenuState;

class StartState : public GameState
{
public:
	StartState(const char* menu_json);
	~StartState();

	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;

	void RollCredits();
	void SplashScreens();
	void OptionsMenu();

	void MainMenu();
	void ConfirmQuit();

	void PushStartState();

private:
	enum class State {
		Menu,
		Splash,
		Credits,
		Options,
		QuitConfirmation
	};

	Vector<GUIObject*> menu_objects;
	Vector<GUIObject*> splash_screen_objects;
	Vector<GUIObject*> credits_objects;
	Vector<GUIObject*> options_objects;
	Vector<GUIObject*> quit_confirmation_objects;

	Int32 menu_back_sound;

	Float32 timer;
	Int32 screen_idx;
	State curr_state;
	bool first_time;
};

extern StartState* p_start_state;

