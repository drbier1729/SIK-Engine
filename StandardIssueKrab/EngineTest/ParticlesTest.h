#pragma once

#include "Test.h"

struct ParticleEmitter;

class ParticlesTest : public Test {
private:
	ParticleEmitter* controllable_emitter;

public:
	void Setup(EngineExport* p_engine_export_struct) override;
	void Run() override;
	void Teardown() override;
};