#pragma once

class GamePlayState;
class GarageState;
class PromptState;
class StartState;

void RegisterLuaPlayerObject(
	sol::state& script_state, GameObject* p_obj, const char* lua_player_name);

void RegisterLuaGamePlayState(
	sol::state& script_state, GamePlayState* p_state);

void RegisterLuaGarageState(
	sol::state& script_state, GarageState* p_state);

void RegisterLuaPromptState(
	sol::state& script_state, PromptState* p_state);

void RegisterLuaStartState(
	sol::state& script_state, StartState* p_state);