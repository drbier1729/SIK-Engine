#pragma once

#include "Engine/Component.h"

// forward decls
class RenderCam;

class CameraFollow : public Component {
public:
	RenderCam* p_cam;

public:
	VALID_COMPONENT(CameraFollow);

	CameraFollow() = default;
	~CameraFollow() noexcept = default;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Update(Float32 dt);
};