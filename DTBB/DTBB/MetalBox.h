#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

class MetalBox : public Component {
public:
	VALID_COMPONENT(MetalBox);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(MetalBox);

	MetalBox();
	~MetalBox();

	
	void OnCollide(GameObject* other) override;
	void Link() override;

	void Reset() override;

	void Update(Float32 dt);

	void AudioUpdate();

private:
	Bool collided_last_frame;
	Bool collided_this_frame;

	Float32 sound_cooldown_timer;

	Vec3 orig_pos;
	Quat orig_ori;
};