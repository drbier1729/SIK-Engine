#pragma once

#include "Engine\Component.h"

class Texture;
class Mesh;
struct ParticleEmitter;

class Destroyable : public Component
{
public:
	VALID_COMPONENT(Destroyable);
	ALLOW_PRIVATE_REFLECTION;

	Destroyable();
	~Destroyable() noexcept;
	
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Link() override;
	void OnCollide(GameObject* other) override;
	void Update(Float32 dt) override;
	bool isDestroyed() const;
	bool isDestroyedThisFrame() const;
	void Enable() override;
	void Reset() override;

	void SayGoodbyeAndDie();

private:
	//Boolean to indicate if the object is destroyed
	Bool to_destroy = false;
	Bool destroyed = false;
	ParticleEmitter* p_hit_emitter;

	//Boolean to indicate if the object was destroyed on the current frame
	Bool destroyed_this_frame = false;
	Texture* alt_tex;
	Mesh* alt_mesh;
	//Threshold momentum that the colliding object needs to reach to trigger the destroyable
	Float32 destroy_momentum;

	//Starting position and orientation of the destroyable to reset
	Vec3 starting_position;
	Quat starting_orientation;

	//Starting texture and mesh
	Texture* starting_tex;
	Mesh* starting_mesh;
};

