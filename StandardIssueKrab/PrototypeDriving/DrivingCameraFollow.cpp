#include "stdafx.h"
#include "DrivingCameraFollow.h"
#include "Engine\GameObjectManager.h"
#include "Engine\GraphicsManager.h"
#include "Engine\RenderCam.h"

DrivingCameraFollow::~DrivingCameraFollow() {
	m_main_camera = nullptr;
}

void DrivingCameraFollow::Deserialize(rapidjson::Value const& json_value) {

}

void DrivingCameraFollow::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {

}

void DrivingCameraFollow::Update(Float32 dt) {
	if (m_main_camera == nullptr) {
		m_main_camera = p_graphics_manager->GetPActiveCam();
	}

	Transform* tr = this->GetOwner()->HasComponent<Transform>();
	if (tr != nullptr)
	{
		m_main_camera->SetCameraCenter(tr->position);
	}
}