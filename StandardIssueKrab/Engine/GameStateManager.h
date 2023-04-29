#pragma once
#include "GameState.h"



class GameStateManager {
public:
	GameStateManager();
	~GameStateManager();

	void Update(Float32 dt);
	void FixedUpdate(Float32 fixed_dt);
	void HandleEvent(SDL_Event const& e);

	void Clear();
	void PushState(GameState* p_state);
	GameState* PopState();
	GameState* GetStackTop() const;

	void TransitionToState(GameState* next_state);
private:
	Vector<GameState*> state_stack;
	static const Uint8 MAX_STATES = 20;
};

extern GameStateManager* p_gamestate_manager;
