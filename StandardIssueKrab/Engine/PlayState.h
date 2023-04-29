#pragma once

#include "GameState.h"

class PlayState : public GameState
{
public:
	PlayState();
	PlayState(String json_to_load);

	virtual void Enter();
	virtual void Exit();
	virtual void Update();


private:
};