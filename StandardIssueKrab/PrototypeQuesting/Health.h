#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

class Health : public Component {
public:
	VALID_COMPONENT(Health);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(Health);

	Health() = default;
	~Health() noexcept = default;

	void Update(Float32 dt);

	void TakeDamage(Int32 damage);

private:
	Int32 health;
};