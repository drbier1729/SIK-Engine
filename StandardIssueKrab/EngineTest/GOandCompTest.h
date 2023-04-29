#pragma once
#include "Test.h"

class GOandCompTest : public Test
{
	void Setup(EngineExport* p_engine_export_struct) override;
	void Run() override;
	void Teardown() override;
};

