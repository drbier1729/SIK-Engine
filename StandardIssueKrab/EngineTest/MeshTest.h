#pragma once
#include "Test.h"
class MeshTest : public Test
{
public:
	void Setup(EngineExport* _p_engine_export_struct) override;

	void Run() override;

	void Teardown() override;
};

