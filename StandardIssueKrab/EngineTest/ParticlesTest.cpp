#include "stdafx.h"
#include "ParticlesTest.h"

#include "Engine/ParticleSystem.h"
#include "Engine/InputManager.h"
#include "Engine/ResourceManager.h"

void ParticlesTest::Setup(EngineExport* p_engine_export_struct) {
	p_particle_system = p_engine_export_struct->p_engine_particle_system;
	p_input_manager = p_engine_export_struct->p_engine_input_manager;
	p_resource_manager = p_engine_export_struct->p_engine_resource_manager;

	ParticleEmitter* emitter = nullptr;
	for (auto i = 0; i < 1; ++i) {
		emitter = p_particle_system->NewEmitter();
		emitter->particles_per_sec = 2000;
		emitter->gravity_scale = 1.0f;
		emitter->uniform_scale_gen_bnds = { .min = 0.02f, .max = 0.1f };
		//emitter->vel_gen_bnds = { .min = Vec3(-1, 0, -1), .max = Vec3(1, 1, 1) };
		emitter->pos_gen_bnds = { .min = Vec3(-0.2f), .max = Vec3(0.2f) };
		emitter->color_gen_bnds = { .min = Vec4(1), .max = Vec4(1) };
		emitter->position = Vec3(0, 3, 0);
		emitter->texture = p_resource_manager->GetTexture("goose.jpg"_sid);
	}
	// Track the last emitter so we can control it with the keyboard
	controllable_emitter = emitter;
	SetRunning();
}

void ParticlesTest::Run() {

	if (p_input_manager->IsKeyTriggered(SDL_SCANCODE_BACKSPACE)) {
		SetPassed();
		SIK_INFO("Exiting particles test");
		return;
	}

	Vec2 move{};
	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_W)) {
		move.y += 1.0f;
	}
	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_S)) {
		move.y -= 1.0f;
	}
	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_A)) {
		move.x -= 1.0f;
	}
	if (p_input_manager->IsKeyPressed(SDL_SCANCODE_D)) {
		move.x += 1.0f;
	}
	controllable_emitter->position += 5.0f * Vec3(move.x, 0, -move.y) * 0.0166f;

	SetRunning();
}

void ParticlesTest::Teardown() {
	p_particle_system->Clear();
	p_particle_system = nullptr;
	SetPassed();
}