#pragma once
#include "Test.h"
class FixedObjectPoolTest : public Test
{
public:
	void Setup(EngineExport*) override;

	void Run() override;

	void Teardown() override;
};

