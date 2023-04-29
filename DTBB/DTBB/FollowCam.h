#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward decarations
class RenderCam;

class FollowCam : public Component {
public:
	VALID_COMPONENT(FollowCam);
	ALLOW_PRIVATE_REFLECTION;

	FollowCam() = default;
	~FollowCam() noexcept = default;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Link() override;
	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);

	void SetEnable(Bool status);

	void Enable() override;
	void Disable() override;
private:
	RenderCam* p_follow_cam;
	Bool enabled = true;
	Bool static_follow = false;
	Vec3 camera_center;
	Float32 leash_radius;
	Float32 camera_speed;
	Float32 default_zoom;
	Bool is_controlled;

	/*
	* Function to handle inputs on the right stick/keyboard 
	* Uses the CarController action map since that's basically the player controller.
	* Updates camera position based on camera_center
	* Handles the gradual movement of camera to/from the player
	* Returns: Void
	*/
	void HandleControls(Float32 dt);

};