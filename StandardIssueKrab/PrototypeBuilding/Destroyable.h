#pragma once

#include "Engine\Component.h"

class Texture;
class Mesh;

class Destroyable : public Component
{
public:
	VALID_COMPONENT(Destroyable);
	~Destroyable() = default;
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void OnCollide(GameObject* other) override;
	void Update(Float32 dt) override;
	bool isDestroyed() const;
	bool isDestroyedThisFrame() const;
private:
	//Boolean to indicate if the object is destroyed
	bool destroyed = false;

	//Boolean to indicate if the object was destroyed on the current frame
	bool destroyed_this_frame = false;
	Texture* alt_tex;
	Mesh* alt_mesh;
};

