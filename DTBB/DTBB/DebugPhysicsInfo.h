#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

class DebugPhysicsInfo : public Component {
public:
	VALID_COMPONENT(DebugPhysicsInfo);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(DebugPhysicsInfo)
	
	DebugPhysicsInfo() = default;
	~DebugPhysicsInfo() = default;
	void Update(Float32 dt) override;

private:
	Array<Vec3, 6> faceNormalsExpected;
	Array<Vec3, 6> faceNormalsComputed;
};

