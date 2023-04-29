#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// Forward Declaration
struct ParticleEmitter;
class GameObject;
class InputAction;

class Turret : public Component {
public:
	VALID_COMPONENT(Turret);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(Turret);

	Turret();
	~Turret() noexcept;

	void Update(Float32 dt);

	InputAction* GetInputAction() const;
	void SetFiring(Bool is_firing);
	void AddEnemy(GameObject* enemy);
	void RemoveEnemy(GameObject* enemy);

	Bool HasEnemies() const;

private:
	ParticleEmitter* p_emitter;
	Vector<GameObject*> enemies;
	InputAction* turret_actions;
	Uint32 kill_count;
};