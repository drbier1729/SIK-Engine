#include "stdafx.h"

#include<assimp/matrix4x4.h>
#include<assimp/matrix3x3.h>
#include<assimp/vector3.h>
#include<assimp/vector2.h>
#include<assimp/quaternion.h>

#include "AssimpHelper.h"

#include <glm/gtc/type_ptr.hpp>

Mat4 AssimpHelper::Mat4Cast(aiMatrix4x4 const& aiMat4) {
	return glm::transpose(glm::make_mat4(&(aiMat4.a1)));
}

Mat4 AssimpHelper::Mat4Cast(aiMatrix3x3 const& aiMat3) {
	return glm::transpose(glm::make_mat4(&(aiMat3.a1)));
}

Vec3 AssimpHelper::Vec3Cast(aiVector3D const& aiVec3) {
	return Vec3{ aiVec3.x, aiVec3.y, aiVec3.z };
}

Vec2 AssimpHelper::Vec2Cast(aiVector2D const& aiVec2) {
	return Vec2{ aiVec2.x, aiVec2.y };
}

Quat AssimpHelper::QuatCast(aiQuaternion const& aiQuat) {
	return Quat{ aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z };
}
