#pragma once

#include "Test.h"

class SerializeTest : public Test
{
public:
	void Setup(EngineExport*) override;

	void Run() override;

	void Teardown() override;
private:

	class GameObject* go;
};

