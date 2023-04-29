#pragma once

// for conversions from assimp data types
class AssimpHelper {
public:
	static Mat4 Mat4Cast(aiMatrix4x4 const& aiMat4);
	static Mat4 Mat4Cast(aiMatrix3x3 const& aiMat3);
	static Vec3 Vec3Cast(aiVector3D const& aiVec3);
	static Vec2 Vec2Cast(aiVector2D const& aiVec2);
	static Quat QuatCast(aiQuaternion const& aiQuat);
};