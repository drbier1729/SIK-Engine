#pragma once

// All quantities in world coordinates, unless otherwise specified
struct MotionProperties
{
	static Vec3 gravity;

	Vec3		prev_position = Vec3(0);
	Quat		prev_orientation = Quat(1, 0, 0, 0);

	Float32		mass = 0.0f;
	Float32		inv_mass = 0.0f;	// 0 --> immovable

	Mat3		inertia = Mat3(0);
	Mat3		inertia_local = Mat3(0);
	Mat3		inv_inertia = Mat3(0);
	Mat3		inv_inertia_local = Mat3(0);

	Vec3		linear_velocity = Vec3(0);
	Vec3		angular_velocity = Vec3(0);

	Vec3		accumulated_force = Vec3(0);
	Vec3		accumulated_torque = Vec3(0);

	Float32		linear_damping = 0.0f; // 0 --> no damping, 1 --> fully damped
	Float32		angular_damping = 0.0f; // 0 --> no damping, 1 --> fully damped

	Float32		gravity_scale = 1.0f; // scales the gravity_vector. 0 --> no gravity.


	// Manipulators
	inline void SetMass(Float32 mass) noexcept;
};



// Inline definitions

inline void MotionProperties::SetMass(Float32 m) noexcept {
	mass = m;
	inv_mass = glm::epsilonEqual(mass, 0.0f, 0.0001f) ? 0.0f : 1.0f / mass;
}
