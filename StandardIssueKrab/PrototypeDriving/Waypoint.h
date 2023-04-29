#pragma once

#include "Engine\Component.h"

class Waypoint : public Component
{
public:
	VALID_COMPONENT(Waypoint);
	~Waypoint();
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void OnCollide(GameObject* other) override;
	
private:
	float radius = 1.0f;
};

