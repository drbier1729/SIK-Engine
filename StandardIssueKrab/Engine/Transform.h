#pragma once

struct Transform {
	Vec3 position = Vec3(0);
	Quat orientation = Quat(1,0,0,0);
	Vec3 scale = Vec3(1);

	inline Mat4 ToMat4() const {
		return glm::translate(Mat4(1), position) * glm::toMat4(orientation) * glm::scale(Mat4(1), scale);
	}

	inline Vec3 EulerAngles() const{
		return glm::eulerAngles(orientation);		
	}

	float GetPositionX() {
		return position.x;
	}

	float GetPositionY() {
		return position.y;
	}

	float GetPositionZ() {
		return position.z;
	}

	float GetScaleX() {
		return scale.x;
	}

	float GetScaleY() {
		return scale.y;
	}

	float GetScaleZ() {
		return scale.z;
	}

	Vec3 LocalToWorld(Vec3 const& local_pt) {
		return orientation * local_pt + position;
	}

	Vec3 WorldToLocal(Vec3 const& world_pt) {
		return glm::inverse(orientation) * (world_pt - position);
	}

	inline void RotateOrientation(float angle) {
		Float32 curr_angle = glm::angle(orientation);

		curr_angle += angle;

		//Clamp the  angle
		if (curr_angle < -1 * glm::pi<Float32>())
			curr_angle = glm::pi<Float32>();
		else if (curr_angle > glm::pi<Float32>())
			curr_angle = -1 * glm::pi<Float32>();


		orientation =
			glm::toQuat(glm::rotate(Mat4{ 1.0f }, curr_angle, Vec3{ 0.0f, 1.0f, 0.0f }));
	}

	inline void SetPosition(float x, float y, float z) {

		position.x = x;
		position.y = y;
		position.z = z;
	}

	inline void SetScale (float x, float y, float z) {

		scale.x = x;
		scale.y = y;
		scale.z = z;
	}

	inline void ToQuat4(float roll, float pitch, float yaw) {	
		Quat quat = glm::rotate(Quat(1, 0, 0, 0), roll, Vec3(1, 0, 0)) *
					glm::rotate(Quat(1, 0, 0, 0), pitch, Vec3(0, 1, 0)) *
					glm::rotate(Quat(1, 0, 0, 0), yaw, Vec3(0, 0, 1));
		orientation = quat;
	}
};