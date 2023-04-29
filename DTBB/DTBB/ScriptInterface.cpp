#include "stdafx.h"

#include "Engine/GameObject.h"
#include "Engine/ScriptingManager.h"
#include "Engine/InputManager.h"

#include "ScriptInterface.h"

#include "GamePlayState.h"
#include "Health.h"
#include "VersusState.h"
#include "Inventory.h"
#include "GarageState.h"
#include "PromptState.h"
#include "StartState.h"

void RegisterLuaPlayerObject(
	sol::state& script_state, GameObject* p_obj, const char* lua_player_name) {
	Health* player_health = p_obj->HasComponent<Health>();
	Transform* player_transform = p_obj->HasComponent<Transform>();
	Inventory* player_inventory = p_obj->HasComponent<Inventory>();

	auto player_lua_obj = script_state[lua_player_name].get_or_create<sol::table>();

	player_lua_obj.set("max_health", player_health->GetMaxHP());
	player_lua_obj.set_function(
		"GetCurrentHealth", &Health::GetCurrHP, player_health);
	player_lua_obj.set_function(
		"TookDamageLastFrame", &Health::TookDamageLastFrame, player_health);

	player_lua_obj.set("position", &player_transform->position);

	player_lua_obj.set_function("GetCollectableCount", &Inventory::GetCollectableCount, player_inventory);
	player_lua_obj.set_function("RemoveCollectables", &Inventory::RemoveCollectables, player_inventory);
}

void RegisterLuaGamePlayState(
	sol::state& script_state, GamePlayState* p_state) {
	auto state_lua_obj = script_state["gameplay_state"].get_or_create<sol::table>();

	state_lua_obj.set("max_turrets", p_state->GetTurretCount());
	state_lua_obj.set("max_destructibles", p_state->GetDestructibleCount());
	state_lua_obj.set("max_small_cars", p_state->GetSmallCarCount());
	state_lua_obj.set("max_big_cars", p_state->GetBigCarCount());

	state_lua_obj.set_function(
		"GetTurretCount", &GamePlayState::GetTurretCount, p_state);
	state_lua_obj.set_function(
		"GetDestructibleCount", &GamePlayState::GetDestructibleCount, p_state);
	state_lua_obj.set_function(
		"GetSmallCarCount", &GamePlayState::GetSmallCarCount, p_state);
	state_lua_obj.set_function(
		"GetBigCarCount", &GamePlayState::GetBigCarCount, p_state);
	
	state_lua_obj.set_function(
		"GetProgress", &GamePlayState::GetProgress, p_state);
	state_lua_obj.set_function(
		"GetPreviousProgress", &GamePlayState::GetPreviousProgress, p_state);
	state_lua_obj.set_function(
		"IsLastLevel", &GamePlayState::IsLastLevel, p_state);

	state_lua_obj.set("PROGRESS_LOSE",     Progress::Lose);
	state_lua_obj.set("PROGRESS_START",    Progress::Start);
	state_lua_obj.set("PROGRESS_WORKING",  Progress::Working);
	state_lua_obj.set("PROGRESS_COMPLETE", Progress::Complete);
}

void RegisterLuaGarageState(
	sol::state& script_state, GarageState* p_state) {
	auto state_lua_obj = script_state["garage_state"].get_or_create<sol::table>();

	state_lua_obj.set_function(
		"HasWreckingBallUpgrade", &GarageState::HasWreckingBallUpgrade, p_state);
	state_lua_obj.set_function(
		"UpgradeWreckingBall", &GarageState::UpgradeWreckingBall, p_state);
	
	state_lua_obj.set_function(
		"CompletedJunkyard", &GarageState::CompletedJunkyard, p_state);
	state_lua_obj.set_function(
		"CompletedConstruction", &GarageState::CompletedConstruction, p_state);
	state_lua_obj.set_function(
		"CompletedMuscleBeach", &GarageState::CompletedMuscleBeach, p_state);

	state_lua_obj.set_function(
		"HasMagnetUpgrade", &GarageState::HasMagnetUpgrade, p_state);
	state_lua_obj.set_function(
		"UpgradeMagnet", &GarageState::UpgradeMagnet, p_state);

	state_lua_obj.set_function(
		"EnterConstructionArena", &GarageState::EnterConstructionArena, p_state);
	state_lua_obj.set_function(
		"EnterMuscleArena", &GarageState::EnterMuscleArena, p_state);
	state_lua_obj.set_function(
		"EnterJunkyardArena", &GarageState::EnterJunkyardArena, p_state);
}

void RegisterLuaPromptState(sol::state& script_state, PromptState* p_state) {
	auto state_lua_obj = script_state["tutorial_state"].get_or_create<sol::table>();

	state_lua_obj.set_function(
		"GetCarAmountMoved", &PromptState::GetCarAmountMoved, p_state);
	state_lua_obj.set_function(
		"GetCarAmountDrifted", &PromptState::GetCarAmountDrifted, p_state);
	state_lua_obj.set_function(
		"GetDistanceToTurret", &PromptState::GetDistanceToTurret, p_state);
}

void RegisterLuaStartState(sol::state& script_state, StartState* p_state) {
	auto state_lua_obj = script_state["start_state"].get_or_create<sol::table>();

	state_lua_obj.set_function(
		"RollCredits", &StartState::RollCredits, p_state);

	state_lua_obj.set_function(
		"OptionsMenu", &StartState::OptionsMenu, p_state);

	state_lua_obj.set_function(
		"MainMenu", &StartState::MainMenu, p_state);
	
	state_lua_obj.set_function(
		"ConfirmQuit", &StartState::ConfirmQuit, p_state);

	state_lua_obj.set_function(
		"PushStart", &StartState::PushStartState, p_state);

}