#pragma once

#include "Engine\Component.h"
#include "Engine\SIKTypes.h"

class RenderCam;

class DrivingCameraFollow : public Component
{
public:
	VALID_COMPONENT(DrivingCameraFollow);
	~DrivingCameraFollow();

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Update(Float32 dt) override;

private:
	RenderCam* m_main_camera;
};