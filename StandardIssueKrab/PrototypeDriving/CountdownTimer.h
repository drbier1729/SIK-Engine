#pragma once
#include "Engine/InputAction.h"
#include "Engine/GameObject.h"

class CountdownTimer {
public:
	CountdownTimer();
	~CountdownTimer();

	void Update(float dt);
	void TimerUpdate(float dt);
	void ResetTimer();
	bool GetTimeState();

private:
	InputAction test_actions{ "default" };

	bool timer_is_start = false;
	bool is_it_game_time = false;
	float game_time;
};

