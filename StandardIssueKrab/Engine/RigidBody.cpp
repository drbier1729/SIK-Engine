#include "stdafx.h"
#include "RigidBody.h"

#include "GameObject.h"
#include "Transform.h"
#include "MotionProperties.h"

RigidBody const RigidBody::world_body = {};

ColliderPair::ColliderPair(RigidBody* a_, Uint32 idx_a_, RigidBody* b_, Uint32 idx_b_)
	: a{ a_ }, idx_a{ idx_a_ }, b{ b_ }, idx_b{ idx_b_ } {
	if (a > b) {
		std::swap(a, b);
		std::swap(idx_a, idx_b);
	}

	SIK_ASSERT(a && b, "Invalid pointers.");
	SIK_ASSERT(idx_a <= a->num_colliders || idx_a == RigidBody::MAX_COLLIDERS, "Invalid collider indices");
	SIK_ASSERT(idx_b <= b->num_colliders || idx_b == RigidBody::MAX_COLLIDERS, "Invalid collider indices");
}


void RigidBody::AddColliders(Collision::Collider* const c[], Uint32 num) {

	SIK_ASSERT(num + num_colliders < MAX_COLLIDERS, "Too many colliders provided.");

	Uint32 const upper_bnd = std::min(num + num_colliders, MAX_COLLIDERS);

	for (auto i = num_colliders; i < upper_bnd; ++i) {
		SIK_ASSERT(c[i], "Collider was null");

		colliders[i] = c[i];
		colliders[i]->SetOwner(this);
	}
	num_colliders += upper_bnd;
}


void RigidBody::AddMotionProperties(MotionProperties* mp) {
	SIK_ASSERT(not motion_props, "This RigidBody already has motion properties.");
	SIK_ASSERT(motion_type != MotionType::Static, "This RigidBody is Static and cannot have motion properties.");

	if (not motion_props) {
		motion_props = mp;
		motion_props->prev_position = position;
		motion_props->prev_orientation = orientation;
	}
}


void RigidBody::AddForce(Vec3 const& force) {
	if (motion_props) {
		motion_props->accumulated_force += force;
	}
	info.set(IS_AWAKE);
}

void RigidBody::AddForceAtWorldPoint(Vec3 const& world_pos, Vec3 const& force) {
	if (motion_props) {
		Vec3 const pt = world_pos - position;
		motion_props->accumulated_force += force;
		motion_props->accumulated_torque += glm::cross(pt, force);
	}
	info.set(IS_AWAKE);
}

void RigidBody::AddForceAtLocalPoint(Vec3 const& local_pos, Vec3 const& force) {
	if (motion_props) {
		motion_props->accumulated_force += force;
		motion_props->accumulated_torque += glm::cross(local_pos, force);
	}
	info.set(IS_AWAKE);
}

void RigidBody::ComputeConstants() {

	UseBoundingBoxAsCollider(num_colliders == 0);

	if (motion_props) {

		MotionProperties& mp = *motion_props;

		// This assumes Colliders have had their transforms set based on RigidBody's local space
		if (num_colliders > 0) {
			mp.mass = 0.0f;
			for (auto i = 0u; i < num_colliders; ++i) {
				SIK_ASSERT(colliders[i], "Collider was null");
				Collision::Collider const& c = *colliders[i];

				Float32 const m = c.mass;
				mp.mass += m;
				centroid_L += m * c.GetRelativePosition();
			}

			mp.inv_mass = mp.mass > 0.0f ? 1.0f / mp.mass : 0.0f;
			centroid_L *= mp.inv_mass;
		
			for (auto i = 0u; i < num_colliders; ++i) {
				SIK_ASSERT(colliders[i], "Collider was null");
				Collision::Collider const& c = *colliders[i];

				mp.inertia_local += c.mass * c.GetInertiaTensorRelative();
			}

			UpdateAABB();
		}
		else {
			UpdateAABB();

			mp.inertia_local = Mat3(0);
			/*Float32 const w2 = local_bounds.halfwidths.x * local_bounds.halfwidths.x;
			Float32 const h2 = local_bounds.halfwidths.y * local_bounds.halfwidths.y;
			Float32 const d2 = local_bounds.halfwidths.z * local_bounds.halfwidths.z;
			mp.inertia_local = 1.0f / 12.0f * Mat3(
				h2 + d2, 0, 0,
				0, w2 + d2, 0,
				0, 0, w2 + h2
			);*/
		}
		mp.inv_inertia_local = Mat3(0);
		//mp.inv_inertia_local = glm::inverse(mp.inertia_local);
	}
	else {
		UpdateAABB();
	}
}


void RigidBody::IntegrateForces(Float32 h) {
	if (not motion_props || motion_type != MotionType::Dynamic) {
		SIK_ASSERT(false, "This can only be called on dynamic rigidbodies");
			return;
	}
	MotionProperties& mp = *motion_props; // alias for readability

	// Update linear velocity and project centroid position
	mp.linear_velocity += h * (MotionProperties::gravity * mp.gravity_scale + mp.accumulated_force * mp.inv_mass);
	mp.linear_velocity *= (1.0f - mp.linear_damping);


	// Update angular velocity and project rotation
	Vec3 const w = mp.accumulated_torque - glm::cross(mp.angular_velocity, mp.inertia * mp.angular_velocity); // gyroscopic term may need to be ignored
	mp.angular_velocity += h * mp.inv_inertia * w;
	mp.angular_velocity *= (1.0f - mp.angular_damping);


	// Zero out forces and torque
	mp.accumulated_force = Vec3(0);
	mp.accumulated_torque = Vec3(0);
}

void RigidBody::IntegrateVelocities(Float32 h) {
	if (not motion_props || motion_type != MotionType::Dynamic) {
		SIK_ASSERT(false, "This can only be called on dynamic rigidbodies");
			return;
	}
	MotionProperties& mp = *motion_props; // alias for readability

	// Position update
	mp.prev_position = position;
	position += h * mp.linear_velocity;
	if (glm::all(glm::epsilonEqual(mp.prev_position, position, 0.0005f))) {
		position = mp.prev_position;
	}

	// Orientation update
	mp.prev_orientation = orientation;
	Quat const dq{ 0, mp.angular_velocity.x , mp.angular_velocity.y, mp.angular_velocity.z };
	orientation += h * 0.5f * dq * orientation;
	orientation = glm::normalize(orientation);
	SIK_ASSERT(not glm::any(glm::isnan(orientation)), "Orientation was NAN");
	if (glm::all(glm::epsilonEqual(mp.prev_orientation, orientation, 0.0005f))) {
		orientation = mp.prev_orientation;
	}
}

void RigidBody::SyncWithOwnerTransform(Float32 t, bool interpolate) {
	if (not owner) { return; }

	Transform* tr = owner->HasComponent<Transform>();

	if (motion_props) {

		// Interpolate
		if (interpolate) {
			tr->position = (1.0f - t) * motion_props->prev_position + t * position;
			tr->orientation = glm::slerp(motion_props->prev_orientation, orientation, t);
		}
		else {
			// Extrapolate
			tr->position = position + motion_props->linear_velocity * t;

			Quat const dq{ 0, motion_props->angular_velocity.x , motion_props->angular_velocity.y, motion_props->angular_velocity.z };
			tr->orientation = orientation + t * 0.5f * dq * orientation;
			tr->orientation = glm::normalize(tr->orientation);

			SIK_ASSERT(not glm::any(glm::isnan(tr->orientation)), "Orientation was NAN");
		}
	}
	else {
		// For static rigidbodies, set position and orientation to Transform values
		position = tr->position;
		orientation = tr->orientation;
		UpdateAABB();
	}

}


void RigidBody::UpdateCentroidFromOwnerPosition() {
	position = glm::toMat3(orientation) * centroid_L + owner->HasComponent<Transform>()->position;
}

Vector<ColliderPair> RigidBody::Intersect(RigidBody* other) {

	using Collision::Collider;

	Vector<ColliderPair> pairs{};

	// First check bounding box around all colliders
	if (not bounds.Intersects(other->bounds)) { return pairs; }

	// Then check bounds of each individual collider
	for (auto i = 0u; i < num_colliders; ++i) {

		Collider* c = colliders[i];
		for (auto j = 0u; j < other->num_colliders; ++j) {
			if (c->BoundsIntersect(other->colliders[i])) {
				pairs.emplace_back(this, i, other, j);
			}
		}
	}

	return pairs;
}

void RigidBody::UpdateInternals() {

	Mat4 const& tr = TransformMatrix();

	for (auto i = 0u; i < num_colliders; ++i) {
		Collision::Collider* c = colliders[i];
		c->UpdateWorldTransformFromBody(tr);
	}

	// // Commented out to disable rotations
	//if (motion_props) {
	//	// Recompute the inertia tensor in world space, if we need to
	//	Mat3 const rot = glm::toMat3(orientation);
	//	motion_props->inertia = rot * motion_props->mass * motion_props->inertia_local * glm::transpose(rot);
	//	motion_props->inv_inertia = glm::inverse(motion_props->inertia);
	//}
}


// Recomputes AABB halfwidths based on colliders' bounds
void RigidBody::UpdateAABB() {
	if (IsBoundingBoxUsedAsCollider()) {
		bounds = local_bounds.Transformed(glm::toMat3(orientation), position);
		return;
	}
	
	Vec3 min{ std::numeric_limits<Float32>::max() };
	Vec3 max{ std::numeric_limits<Float32>::lowest() };

	for (auto i = 0u; i < num_colliders; ++i) {
		Collision::Collider* c = colliders[i];

		Collision::AABB aabb = c->GetBoundingBox();
		min = glm::min(min, aabb.position - aabb.halfwidths);
		max = glm::max(max, aabb.position + aabb.halfwidths);
	}

	bounds.SetMinMax(min, max);
}

BEGIN_ATTRIBUTES_FOR(RigidBodyCreationSettings)
DEFINE_MEMBER(Vec3, position)
DEFINE_MEMBER(Quat, orientation)
DEFINE_MEMBER(Vec3, aabb_halfwidths)
DEFINE_MEMBER(Float32, gravity_scale)
DEFINE_MEMBER(Float32, linear_damping)
DEFINE_MEMBER(Float32, angular_damping)
DEFINE_MEMBER(Float32, friction)
DEFINE_MEMBER(Float32, restitution)
DEFINE_MEMBER(Float32, mass)
DEFINE_MEMBER(Bool, is_enabled)
DEFINE_MEMBER(Bool, is_trigger)
DEFINE_MEMBER(Bool, use_aabb_as_collider)
END_ATTRIBUTES

BEGIN_ATTRIBUTES_FOR(RigidBody)
DEFINE_MEMBER(Vec3, position)
DEFINE_MEMBER(Quat, orientation)
DEFINE_MEMBER(Float32, friction)
DEFINE_MEMBER(Float32, restitution)
END_ATTRIBUTES

#pragma region OLD XPBD STUFF
//
//void RigidBody::UpdatePositions(Float32 h) {
//	if (not motion_props || motion_type != MotionType::Dynamic) {
//		SIK_ASSERT(false, "This can only be called on dynamic rigidbodies")
//			return;
//	}
//	MotionProperties& mp = *motion_props; // alias for readability
//
//	// Update linear velocity and project centroid position
//	mp.prev_position = position;
//	mp.linear_velocity += h * mp.accumulated_force * mp.inv_mass;
//
//	position += h * mp.linear_velocity;
//
//
//	// Update angular velocity and project rotation
//	mp.prev_orientation = orientation;
//	Vec3 const w = mp.accumulated_torque - glm::cross(mp.angular_velocity, mp.inertia * mp.angular_velocity);
//	mp.angular_velocity += h * mp.inv_inertia * w;
//
//	Quat const dq{ 0, mp.angular_velocity.x , mp.angular_velocity.y, mp.angular_velocity.z };
//	orientation += h * 0.5f * dq * orientation;
//	orientation = glm::normalize(orientation);
//	SIK_ASSERT(not glm::any(glm::isnan(orientation)), "Orientation was NAN");
//
//
//	// Zero out forces and torque, then apply gravity
//	mp.accumulated_force = mp.gravity_scale * MotionProperties::gravity * mp.mass;
//	mp.accumulated_torque = Vec3(0);
//
//
//	// Recompute the inertia tensor in world space
//	Mat3 const rot = glm::toMat3(orientation);
//	mp.inertia = rot * mp.inertia_local * glm::transpose(rot);
//	mp.inv_inertia = glm::inverse(mp.inertia);
//}
//
//void RigidBody::UpdateVelocities(Float32 h) {
//	if (not motion_props || motion_type != MotionType::Dynamic) {
//		SIK_ASSERT(false, "This can only be called on dynamic rigidbodies");
//		return;
//	}
//	MotionProperties& mp = *motion_props; // alias for readability
//
//	// Compute velocities based on corrected position and orientation
//	mp.linear_velocity = (position - mp.prev_position) / h;
//
//	Quat const dq = orientation * glm::inverse(mp.prev_orientation);
//	mp.angular_velocity =  2.0f * Vec3(dq.x, dq.y, dq.z) / h;
//	if (dq.w < 0.0f) {
//		mp.angular_velocity *= -1.0f;
//	}
//}
#pragma endregion