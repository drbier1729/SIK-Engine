#include "stdafx.h"
#include "GameStateManager.h"
#include "MemoryManager.h"

void GameStateManager::Clear() {
	state_stack.clear();
}

void GameStateManager::PushState(GameState* p_state) {
	if (state_stack.size() > 0)
		state_stack.back()->Exit();

	state_stack.push_back(p_state);
	state_stack.back()->Enter();
}

GameState* GameStateManager::PopState() {
	GameState* popped_state = state_stack.back();
	state_stack.pop_back();
	popped_state->Exit();
	
	if (state_stack.size() > 0)
		state_stack.back()->Enter();

	return popped_state;
}

GameState* GameStateManager::GetStackTop() const {
	if (state_stack.size() > 0)
		return state_stack.back();
	else
		return nullptr;
}

void GameStateManager::TransitionToState(GameState* next_state) {
	PopState();
	PushState(next_state);
}

GameStateManager::GameStateManager() {
	state_stack.reserve(MAX_STATES);
}

GameStateManager::~GameStateManager() {
	Clear();
}

void GameStateManager::Update(Float32 dt)  {
	if (state_stack.empty())
		return;

	state_stack.back()->Update(dt);
}

void GameStateManager::FixedUpdate(Float32 fixed_dt) {
	if (state_stack.empty())
		return;

	state_stack.back()->FixedUpdate(fixed_dt);
}

void GameStateManager::HandleEvent(SDL_Event const& e) {
	if (state_stack.empty())
		return;
	
	state_stack.back()->HandleEvent(e);
}
