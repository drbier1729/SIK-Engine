#include "stdafx.h"
#include "CollisionArbiter.h"

#include "RigidBody.h"
#include "CollidablePrimitives.h"
#include "MotionProperties.h"
#include "GameObject.h"

// This code is based upon Box2D Lite by Erin Catto... See https://github.com/erincatto/box2d-lite/
// Box 2D Lite copyright:

/*
* Copyright (c) 2006-2009 Erin Catto http://www.gphysics.com
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Erin Catto makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

inline static Vec3 SafeNormalize(Vec3 const& v) {
	if (glm::length2(v) < 0.0001f) {
		return Vec3(1,0,0);
	}
	return glm::normalize(v);
}

CollisionArbiter::CollisionArbiter(ColliderPair const& pair_)
	: pair{pair_}, 
	manifold{},
	friction{ 0 }, restitution{ 0 }
{
	friction = glm::sqrt(pair.a->friction * pair.b->friction);
	restitution = glm::min(pair.a->restitution, pair.b->restitution);

	if (pair.a->IsBoundingBoxUsedAsCollider() || pair.b->IsBoundingBoxUsedAsCollider()) {
		Collision::AABB const obbA{ .position = pair.a->position, .halfwidths = pair.a->local_bounds.halfwidths };
		Collision::AABB const obbB{ .position = pair.b->position, .halfwidths = pair.b->local_bounds.halfwidths };
		manifold = obbA.CollideAsOBB(pair.a->orientation, obbB, pair.b->orientation);
		//manifold = pair.a->bounds.Collide(pair.b->bounds); //Collide axis-aligned boxes
	}
	else {
		SIK_ASSERT(pair.idx_a < RigidBody::MAX_COLLIDERS && pair.idx_b < RigidBody::MAX_COLLIDERS, "Indices out of range.");
		manifold = pair.a->colliders[pair.idx_a]->Collide(pair.b->colliders[pair.idx_b]);
	}
}

void CollisionArbiter::Update(Collision::ContactManifold const& new_manifold) {
	using Collision::Contact;
	using Collision::ContactManifold;

	// Merge existing contacts with new ones
	Contact merged[ContactManifold::MAX_CONTACTS] = {};

	for (Uint32 i = 0; i < new_manifold.num_contacts; ++i) {
		Contact const& new_contact = new_manifold.contacts[i];
		Int32 k = -1;

		for (Uint32 j = 0; j < manifold.num_contacts; ++j) {
			if (glm::distance2(new_contact.position, manifold.contacts[j].position) < 0.01f) { // NOTE THIS IS NOT USING "FEATURES" AND MIGHT NOT WORK ALL THE TIME
				k = j;
				break;
			}
		}

		if (k > -1) { // We found an existing contact and are updating it

			Contact& old_contact = manifold.contacts[k];
			merged[i] = new_contact;

			// Warm starting
			merged[i].impulse_n = old_contact.impulse_n;
			merged[i].impulse_t = old_contact.impulse_t;
		}
		else { // We didn't find an existing contact, so we add the new one
			merged[i] = new_contact;
		}
	}

	for (Uint32 i = 0; i < new_manifold.num_contacts; ++i) {
		manifold.contacts[i] = merged[i];
	}

	manifold.num_contacts = new_manifold.num_contacts;
	manifold.normal = new_manifold.normal;
}

void CollisionArbiter::PreStep(Float32 time_step) {
	using Collision::Contact;
	using Collision::ContactManifold;
	using Collision::Collider;
	
	static constexpr float allowed_penetration = 0.01f;
	static constexpr float bias_factor = 0.2f;

	if (manifold.num_contacts == 0) { return; }
	if (glm::epsilonEqual(time_step, 0.0f, 0.00001f)) { return; }

	Float32 inv_dt = 1.0f / time_step;

	// Default values for motion properties that can be thrown away in case
	// one of the RigidBodies does not have motion_props
	MotionProperties temp0{}, temp1{};

	RigidBody* a = pair.a;
	RigidBody* b = pair.b;
	if (not a || not b || a->IsTrigger() || b->IsTrigger()) { return; }
	if (not a->IsEnabled() || not b->IsEnabled()) { return; }

	MotionProperties* a_mp = (a->motion_props) ? a->motion_props : &temp0;
	MotionProperties* b_mp = (b->motion_props) ? b->motion_props : &temp1;

	Vec3 const& normal = manifold.normal;

	for (Uint32 i = 0; i < manifold.num_contacts; ++i) {
		Contact& c = manifold.contacts[i];

		c.ra = c.position - a->position;
		c.rb = c.position - b->position;
		
		// Compute normal mass
		Vec3 ra_X_n = glm::cross(c.ra, normal);
		Vec3 rb_X_n = glm::cross(c.rb, normal);
		Float32 k_n = a_mp->inv_mass + b_mp->inv_mass;
		k_n += glm::dot(normal, a_mp->inv_inertia * glm::cross(ra_X_n, c.ra) + b_mp->inv_inertia * glm::cross(rb_X_n, c.rb));
		c.mass_n = k_n > 0.0f ? 1.0f / k_n : 0.0f;

		//// Compute tangent mass
		Vec3 rel_vel = b_mp->linear_velocity + glm::cross(b_mp->angular_velocity, c.rb) - (a_mp->linear_velocity + glm::cross(a_mp->angular_velocity, c.ra));
		if (glm::epsilonEqual(glm::length2(rel_vel), 0.0f, 0.001f)) { continue; }

		Vec3 tangent = SafeNormalize(rel_vel - glm::dot(rel_vel, normal)*normal);

		Vec3 ra_X_t = glm::cross(c.ra, tangent);
		Vec3 rb_X_t = glm::cross(c.rb, tangent);
		Float32 k_t = a_mp->inv_mass + b_mp->inv_mass;
		k_t += glm::dot(tangent, a_mp->inv_inertia * glm::cross(ra_X_t, c.ra) + b_mp->inv_inertia * glm::cross(rb_X_t, c.rb));
		c.mass_t = k_t != 0.0f ? 1.0f / k_t : 0.0f;

		// Compute bias
		c.bias = -bias_factor * inv_dt * glm::min(0.0f, -c.penetration + allowed_penetration);

		// Integrate velocities as we accumulate impulses
		Vec3 p = c.impulse_n * normal /* + c.impulse_t * tangent*/;

		a_mp->linear_velocity -= a_mp->inv_mass * p;
		a_mp->angular_velocity -= a_mp->inv_inertia * glm::cross(c.ra, p);

		b_mp->linear_velocity += b_mp->inv_mass * p;
		b_mp->angular_velocity += b_mp->inv_inertia * glm::cross(c.rb, p);

		SIK_ASSERT(not glm::any(glm::isnan(a_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(a_mp->angular_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->angular_velocity)), "NAN");
	}
}

void CollisionArbiter::ApplyImpulse() {
	using Collision::Contact;
	using Collision::ContactManifold;

	if (manifold.num_contacts == 0) { return; }

	// Default values for motion properties that can be thrown away in case
	// one of the RigidBodies does not have motion_props
	MotionProperties temp0{}, temp1{};

	RigidBody* a = pair.a;
	RigidBody* b = pair.b;
	if (not a || not b || a->IsTrigger() || b->IsTrigger()) { return; }
	if (not a->IsEnabled() || not b->IsEnabled()) { return; }

	MotionProperties* a_mp = (a->motion_props) ? a->motion_props : &temp0;
	MotionProperties* b_mp = (b->motion_props) ? b->motion_props : &temp1;

	Vec3 const& normal = manifold.normal;

	for (Uint32 i = 0; i < manifold.num_contacts; ++i) {
		Contact& c = manifold.contacts[i];

		Vec3 rel_vel = b_mp->linear_velocity + glm::cross(b_mp->angular_velocity, c.rb) - (a_mp->linear_velocity + glm::cross(a_mp->angular_velocity, c.ra));

		// Compute and apply normal impulse
		Float32 vn = glm::dot(rel_vel, normal);
		Float32 d_impulse_n = c.mass_n * (-vn + c.bias);

		// Accumulate impulse
		Float32 temp_impulse_n = c.impulse_n;
		c.impulse_n = std::max(temp_impulse_n + d_impulse_n, 0.0f);
		d_impulse_n = c.impulse_n - temp_impulse_n;

		Vec3 p_n = d_impulse_n * normal;

		a_mp->linear_velocity -= a_mp->inv_mass * p_n;
		a_mp->angular_velocity -= a_mp->inv_inertia * glm::cross(c.ra, p_n);

		b_mp->linear_velocity += b_mp->inv_mass * p_n;
		b_mp->angular_velocity += b_mp->inv_inertia * glm::cross(c.rb, p_n);

		SIK_ASSERT(not glm::any(glm::isnan(a_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(a_mp->angular_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->angular_velocity)), "NAN");

		// Recompute relative velocity
		rel_vel = b_mp->linear_velocity + glm::cross(b_mp->angular_velocity, c.rb) - (a_mp->linear_velocity + glm::cross(a_mp->angular_velocity, c.ra));
		
		// Compute and apply tangent impulse
		Vec3 tangent = SafeNormalize(rel_vel - glm::dot(rel_vel, normal) * normal);
		Float32 vt = glm::dot(rel_vel, tangent);

		Float32 d_impulse_t = c.mass_t * (-vt);
		Float32 max_impulse_t = friction * c.impulse_n;

		// Accumulate impulse
		Float32 temp_impulse_t = c.impulse_t;
		c.impulse_t = glm::clamp(temp_impulse_t + d_impulse_t, -max_impulse_t, max_impulse_t);
		d_impulse_t = c.impulse_t - temp_impulse_t;
		
		Vec3 p_t = d_impulse_t * tangent;

		a_mp->linear_velocity -= a_mp->inv_mass * p_t;
		a_mp->angular_velocity -= a_mp->inv_inertia * glm::cross(c.ra, p_t);

		b_mp->linear_velocity += b_mp->inv_mass * p_t;
		b_mp->angular_velocity += b_mp->inv_inertia * glm::cross(c.rb, p_t);

		SIK_ASSERT(not glm::any(glm::isnan(a_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->linear_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(a_mp->angular_velocity)), "NAN");
		SIK_ASSERT(not glm::any(glm::isnan(b_mp->angular_velocity)), "NAN");
	}
}

