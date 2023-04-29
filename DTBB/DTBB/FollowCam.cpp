#include "stdafx.h"

#include "Engine/InputAction.h"

#include "FollowCam.h"
#include "CarController.h"

#include "Engine/RenderCam.h"
#include "Engine/GameObject.h"
#include "Engine/GraphicsManager.h"
#include "Engine/InputAction.h"
#include "Engine/MotionProperties.h"

void FollowCam::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<FollowCam>(json_value, this);

	p_follow_cam = p_graphics_manager->GetPActiveCam();
}

void FollowCam::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<FollowCam>(json_value, this, alloc);
}

void FollowCam::Link()
{
}

void FollowCam::OnCollide(GameObject* other)
{
}

void FollowCam::Update(Float32 dt) {
	SIK_ASSERT(p_follow_cam != nullptr, "No camera");
	
	if (!enabled)
		return;

	HandleControls(dt);
	p_follow_cam->SetCameraCenter(camera_center);
}

void FollowCam::SetEnable(Bool status) {
	enabled = status;
}

void FollowCam::Enable() {
	enabled = true;
	Transform* tr = GetOwner()->HasComponent<Transform>();
	camera_center = tr->position;
}

void FollowCam::Disable() {
	enabled = false;
}

void FollowCam::HandleControls(Float32 dt) {
	CarController* car_controller = GetOwner()->HasComponent<CarController>();
	InputAction& action_map = car_controller->GetActionMap();
	Transform* tr = GetOwner()->HasComponent<Transform>();

	//If we have static follow then just set the position.
	if (static_follow) {
		camera_center = tr->position;
		return;
	}

	is_controlled = false;

	Vec3 right_dir = glm::normalize(p_follow_cam->GetCameraRight());
	Vec3 look_dir = p_follow_cam->GetLookDirection();
	Vec3 look_dir_proj = glm::normalize(Vec3(look_dir.x, 0, look_dir.z));

	Vec3 diff_vector = (tr->position - camera_center);
	Float32 distance_ratio = glm::clamp(glm::length(diff_vector) / leash_radius, 0.0f, 1.0f);
	Float32 dx = 0, dy = 0;
	if (action_map.IsActionPressed(InputAction::Actions::RIGHT_ALT)) {
		is_controlled = true;
		dx = camera_speed * (1 - distance_ratio);
	}
	else if (action_map.IsActionPressed(InputAction::Actions::LEFT_ALT)) {
		is_controlled = true;
		dx = -camera_speed * (1 - distance_ratio);
	}
	right_dir *= (dt * dx);

	if (action_map.IsActionPressed(InputAction::Actions::UP_ALT)) {
		is_controlled = true;
		dy = camera_speed * (1 - distance_ratio);
	}
	else if (action_map.IsActionPressed(InputAction::Actions::DOWN_ALT)) {
		is_controlled = true;
		dy = -camera_speed * (1 - distance_ratio);
	}
	look_dir_proj *= (dt * dy);

	//Handle the movement based on the controls
	camera_center += right_dir;
	camera_center += look_dir_proj;

	if (glm::length(diff_vector) == 0.0f)
		return;

	//Handle the movement based on the leash
	camera_center += (glm::normalize(diff_vector) * camera_speed * (distance_ratio) * dt);

	if (not is_controlled) {
		//If the camera is not being controlled, move the camera back to the car faster
		camera_center += (glm::normalize(diff_vector) * camera_speed * (distance_ratio * 10.0f) * dt);
	}

}

BEGIN_ATTRIBUTES_FOR(FollowCam)
	DEFINE_MEMBER(Float32, leash_radius)
	DEFINE_MEMBER(Float32, camera_speed)
	DEFINE_MEMBER(Bool, static_follow)
	DEFINE_MEMBER(Float32, default_zoom)
END_ATTRIBUTES