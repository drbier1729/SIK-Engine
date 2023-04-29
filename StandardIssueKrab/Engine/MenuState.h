#pragma once

#include "GameState.h"

class MenuState : public GameState
{
public:
	MenuState();
	MenuState(String json_to_load);

	virtual void Enter();
	virtual void Exit();
	virtual void Update();

	~MenuState();

private:
};