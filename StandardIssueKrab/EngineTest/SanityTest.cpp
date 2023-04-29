#include "stdafx.h"

#include "SanityTest.h"
#include "Engine/GameManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputManager.h"

void SanityTest::Setup(EngineExport* _p_engine_export_struct) {
	p_game_manager = _p_engine_export_struct->p_engine_game_manager;
	p_input_manager = _p_engine_export_struct->p_engine_input_manager;
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	SetRunning();
	return;
}

void SanityTest::Run() {
	SIK_INFO("Setting level to 0");
	p_game_manager->SetLevel(0);
	Int32 curr_level = p_game_manager->GetLevel();
	if (curr_level != 0) {
		SIK_ERROR("Expected Level 0. Got Level : \"{}\"", curr_level);
		SetFailed();
		return;
	}
	SIK_INFO("Level set to 0 succesfully");

	SIK_INFO("Setting level to 10");
	p_game_manager->SetLevel(10);
	curr_level = p_game_manager->GetLevel();
	if (curr_level != 10) {
		SIK_ERROR("Expected Level 10. Got Level : \"{}\"", curr_level);
		SetFailed();
		return;
	}
	SIK_INFO("Level set to 10 succesfully");

	SIK_INFO("Test Passed");

	std::random_device rd;
	auto seed_data = std::array<int, std::mt19937::state_size> {};
	std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
	std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
	std::mt19937 generator(seq);
	uuids::uuid_random_generator gen{ generator };

	uuids::uuid const id = gen();
	
	SetPassed();
	return;
}

void SanityTest::Teardown() {
	SIK_INFO("Resetting level to 0");
	p_game_manager->SetLevel(0);
	SetPassed();
	return;
}