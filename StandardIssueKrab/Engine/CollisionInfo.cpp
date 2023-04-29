#include "stdafx.h"
#include "CollisionInfo.h"
//
//#include "RigidBody.h"
//#include "CollisionProperties.h"
//
//// This requires both a_ and b_ to be non-null
//ContactBatch::ContactBatch(RigidBody* a_, RigidBody* b_)
//	: a{ a_ },
//	b{ b_ },
//	friction{ glm::sqrt(a_->collision_props->friction * b_->collision_props->friction) },
//	restitution{ glm::min(a_->collision_props->restitution, b_->collision_props->restitution) },
//	contacts{},
//	num_contacts{ 0 },
//	flags{0}
//{
//	if (a > b) {
//		a = b_;
//		b = a_;
//		flags.set(CB_FLAGS_INVERT_NORMAL);
//	}
//
//	SIK_ASSERT(a != b, "No self-contacts allowed for RigidBodies.");
//	SIK_ASSERT(a != nullptr && b != nullptr, "At least one RigidBody was nullptr.");
//}