#pragma once

// forward declaration
class RenderCam;
class GameObject;
class Turret;
class UniformRandFloat32;

class PrototypeCombat {
public:
	PrototypeCombat();
	~PrototypeCombat();

	// registers game side component functions used by scripts
	void SetupPrototype();

	// update prototype
	void UpdatePrototype();

private:
	PolymorphicAllocator alloc;
	RenderCam* camera;
	GameObject* player_go;
	Turret* p_turret;
	Uint32 enemy_wave;
	UniformRandFloat32* rand_flt_gen;

private:
	// spawns a enemy game object
	void SpawnEnemyWave(/*wave details*/);
};