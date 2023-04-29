#pragma once

#include "Test.h"

class MemoryResourcesTest : public Test
{
public:
	void Setup(EngineExport*) override;

	void Run() override;

	void Teardown() override;

private:
	static constexpr SizeT buf_size = 8 * 1024;
	Array<Byte, buf_size> buffer;
};

