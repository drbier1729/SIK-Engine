#include "stdafx.h"
#include "DebugPhysicsInfo.h"

#include "Engine/GameObject.h"
#include "Engine/RigidBody.h"
#include "Engine/Collision.h"

void DebugPhysicsInfo::Update(Float32 dt) {
	using namespace Collision;

	if (auto* owner = GetOwner(); owner) {
		if (auto* rb = owner->HasComponent<RigidBody>(); rb) {
			
			auto const& o = rb->orientation;
			faceNormalsExpected[0] = o *  Vec3(1, 0, 0);
			faceNormalsExpected[1] = o *  Vec3(0, 1, 0);
			faceNormalsExpected[2] = o *  Vec3(0, 0, 1);
			faceNormalsExpected[3] = o * -Vec3(1, 0, 0);
			faceNormalsExpected[4] = o * -Vec3(0, 1, 0);
			faceNormalsExpected[5] = o * -Vec3(0, 0, 1);

			AABB const& b = rb->local_bounds;
			faceNormalsComputed[0] = o * b.FaceNormal(AABB::Face::Right);
			faceNormalsComputed[1] = o * b.FaceNormal(AABB::Face::Top);
			faceNormalsComputed[2] = o * b.FaceNormal(AABB::Face::Front);
			faceNormalsComputed[3] = o * b.FaceNormal(AABB::Face::Left);
			faceNormalsComputed[4] = o * b.FaceNormal(AABB::Face::Bottom);
			faceNormalsComputed[5] = o * b.FaceNormal(AABB::Face::Top);
		}
	}
}

BEGIN_ATTRIBUTES_FOR(DebugPhysicsInfo)
END_ATTRIBUTES