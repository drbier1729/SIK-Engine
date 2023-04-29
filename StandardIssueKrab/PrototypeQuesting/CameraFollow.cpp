#include "stdafx.h"

#include "CameraFollow.h"

#include "Engine/RenderCam.h"
#include "Engine/GameObject.h"

void CameraFollow::Deserialize(rapidjson::Value const& json_value) {

}

void CameraFollow::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {

}

void CameraFollow::Update(Float32 dt) {
	SIK_ASSERT(p_cam != nullptr, "No camera");

	Transform* tr = GetOwner()->HasComponent<Transform>();
	if (tr != nullptr) {
		p_cam->SetCameraCenter(tr->position);
	}
}