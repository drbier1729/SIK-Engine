#pragma once
#include "Engine/GameState.h"

class GUIObject;

class MenuState : public GameState {
public:
	MenuState(const char* ui_json);
	~MenuState();

	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;

private:
	Vector<GUIObject*> menu_objects;
};

