#pragma once

#include "FixedObjectPool.h"
#include "Collision.h"
#include "CollisionArbiter.h"
#include "MotionProperties.h"
#include "RigidBody.h"
#include "BVHierarchy.h"

#include "FrameTimer.h"

// TODO : remove when possible
#include "Line.h"
#include "CollisionDebugDrawing.h"

class RenderCam;
struct PhysDebugBox;
class Arrow;


struct RayCastHit
{
	Collision::Ray::CastResult info   = {};
	GameObject*                object = nullptr;
};


template<class Fn>
concept RigidBodyQuery = std::predicate<Fn, RigidBody&>;


// Note: because of the large internal buffer, this class should always be
// allocated on the heap.

class PhysicsManager
{
	////////////////////////////////////////////////////////////////////////////
	// TYPES
	////////////////////////////////////////////////////////////////////////////
public:
	static constexpr SizeT MAX_BODIES = 4096;

	template<class T>
	using Pool = FixedObjectPool<T, MAX_BODIES>;

	struct ColliderPools {
		Pool<Collision::Sphere> spheres;
		Pool<Collision::Capsule> capsules;
		Pool<Collision::Hull>  hulls;

		inline Collision::Collider* Add(ColliderCreationSettings const& params, Collision::BVHierarchy& tree) {
			using Collision::Collider;

			Collision::Collider* col = nullptr;

			switch (params.type) {

			break; case Collider::Type::Sphere: {
				auto* s = spheres.insert();
				s->mass = params.mass;
				s->SetRadius(params.sphere_args.radius);
				s->SetRelativePosition(params.position_offset);
				s->SetRelativeRotation(glm::toMat3(params.orientation_offset));
				col = s;
			}
			break; case Collider::Type::Capsule: {
				auto* c = capsules.insert();
				c->mass = params.mass;
				c->SetRadius(params.capsule_args.radius);
				c->SetLength(params.capsule_args.length);
				c->SetRelativePosition(params.position_offset);
				c->SetRelativeRotation(glm::toMat3(params.orientation_offset));
				col = c;
			}
			break; case Collider::Type::Hull: {
				Collision::Hull* h = nullptr;
				if (params.hull_args.is_box) {
					h = hulls.insert(Collision::Hull::BoxInstance(params.hull_args.halfwidths));
				}
				else {
					h = hulls.insert();
				}
				h->mass = params.mass;
				h->SetRelativePosition(params.position_offset);
				h->SetRelativeRotation(glm::toMat3(params.orientation_offset));
				col = h;
			}
			break; default: {
				SIK_ASSERT(false, "Invalid Collidable type.");
			}
			}

			return col;
		}

		inline void Remove(Collision::Collider* p_shape) {
			using Collision::Collider;

			switch (p_shape->GetType()) {

			break; case Collider::Type::Sphere: {
				spheres.erase(static_cast<Collision::Sphere*>(p_shape));
			}
			break; case Collider::Type::Capsule: {
				capsules.erase(static_cast<Collision::Capsule*>(p_shape));
			}
			break; case Collider::Type::Hull: {
				hulls.erase(static_cast<Collision::Hull*>(p_shape));
			}
			break; default: {
				SIK_ASSERT(false, "Invalid Collidable type.");
			}
			}
		}

		void Clear() noexcept {
			spheres.clear();
			capsules.clear();
			hulls.clear();
		}
	};

private:
	
	////////////////////////////////////////////////////////////////////////////
	// DATA
	////////////////////////////////////////////////////////////////////////////
private:
	// Storage for rigidbodies
	Pool<RigidBody>									  rigidbodies;

	// Pointers to all active dynamic rigidbodies. These are the only bodies
	// iterated over during collision detection and resolution.
	Vector<RigidBody*>								  dynamic_bodies;

	// Data required to move dynamic bodies
	Pool<MotionProperties>							  motion_properties;

	// All collision primitives used for narrowphase collision detection
	// and resolution.
	ColliderPools									  colliders;

	// Dynamic AABB hierarchy and supporting data for broadphase 
	// collision detection
	Collision::BVHierarchy							  bvh_tree;
	UnorderedMap<RigidBody*, Collision::BVHandle>     bv_handles;
	Vector<Tuple<RigidBody*, Collision::BVHandle>>    moved_last_frame;
	Vector<ColliderPair>							  broad_phase_results;

	// Narrow phase
	Map<ColliderPair, CollisionArbiter>				  arbiters;

	// Debug drawing: stored in order of colliders
	Vector< std::pair<RigidBody*, Vector<Collision::WireframeMesh>> > debug_wireframes;
	Collision::WireframeMesh cn_plane_mesh;

	// Toggle collisions on/off
	Bool collisions_active = true;

////////////////////////////////////////////////////////////////////////////
// CTORS + DTOR
////////////////////////////////////////////////////////////////////////////
public:
	PhysicsManager();
	~PhysicsManager() noexcept = default;

	// Non-copyable and non-movable
	PhysicsManager(const PhysicsManager&) = delete;
	PhysicsManager& operator=(const PhysicsManager&) = delete;
	PhysicsManager(PhysicsManager&&) = delete;
	PhysicsManager& operator=(PhysicsManager&&) = delete;

	////////////////////////////////////////////////////////////////////////////
	// MANIPULATORS
	////////////////////////////////////////////////////////////////////////////
public:

	/*
	* Performs collision detection and resolution. Reads from and Writes to
	* the rigidbodies arrays. Performs as many fixed-time substeps as possible
	* within the accumulated time. integration_substeps calls to Integrate
	* are called per collision step.
	*/
	/*void Update(FrameTimer::duration_type& accumulated_sim_time, 
		FrameTimer::duration_type collision_step_time,
		Uint32 integration_substeps);*/

	void Update(Float32 time_step);

	// Returns pointer to newly created rigidbody
	RigidBody* CreateRigidBody(RigidBodyCreationSettings const& settings);

	// Returns ptr to next active rigidbody. Removes the rigidbody and all its
	// sub-properties from the PhysicsManager. Sets owner->rigidbody = nullptr.
	void RemoveRigidBody(RigidBody* rb);
	RigidBody* RemoveRigidBodyInternal(RigidBody* rb);

	// Returns first game object that the ray intersects with, plus information
	// about the intersection (i.e. distance from ray origin, etc). Note that
	// this is not a very granular check! It only checks against AABBs.
	RayCastHit RayCast(Collision::Ray const& r);

	void ForEachInRadius(Float32 radius, Vec3 const& center, RigidBodyQuery auto&& function);
	void ForEachInBox(Collision::AABB const& box,            RigidBodyQuery auto&& function);

	void Clear() noexcept;
	void RemoveTombstoned() noexcept;

	void ToggleCollisions();

	////////////////////////////////////////////////////////////////////////////
	// Helpers
	////////////////////////////////////////////////////////////////////////////
private:	
	// Checks for overlapping bounding volumes
	void DetectCollisionsBroad() noexcept;

	void DetectCollisionsBroad_v2() noexcept;

	// Checks for actual collision between physics geometries
	void DetectCollisionsNarrow() noexcept;

	void DetectCollisionsNarrow_v2() noexcept;

	// Resolves collisions and other constraints
	//void XPBDSolvePositions();
	//void XPBDSolveVelocities(Float32 h);
	void SolveGroundConstraint() noexcept;

	////////////////////////////////////////////////////////////////////////////
	// ACCESSORS
	////////////////////////////////////////////////////////////////////////////
public:

	/* 
	* This prevents stutter that would be caused otherwise by the render time and simulation time
	* getting out of sync. See http://gameprogrammingpatterns.com/game-loop.html 
	* for more details. This method reads from rigidbodies but only writes to
	* out_rb_transforms.
	*/
	void Extrapolate(Float32 extrapolation) noexcept;	
	void Interpolate(Float32 interpolation) noexcept;	

	// TODO : temporary. move to the appropriate place when communication between 
	// physics, graphics, and game objects is worked out
	void DebugDraw(GLuint shader_id, const RenderCam* cam, Float32 extrapolation, PhysDebugBox const& box) noexcept;
	void DebugDraw_Forces(GLuint shader_id, const RenderCam* cam, Float32 extrapolation, Arrow const& arrow) noexcept;
};

extern PhysicsManager* p_physics_manager;



void PhysicsManager::ForEachInRadius(Float32 radius, Vec3 const& center, RigidBodyQuery auto&& function)
{
	auto bvQuery = [&function](Collision::BVHNode const& node) {
		RigidBody* p_rb = static_cast<RigidBody*>(node.bv.userData);
		SIK_ASSERT(p_rb, "RigidBody was nullptr");
		return function(*p_rb);
	};

	bvh_tree.Query(radius, center, bvQuery);
}

void PhysicsManager::ForEachInBox(Collision::AABB const& box, RigidBodyQuery auto&& function)
{
	auto bvQuery = [&function](Collision::BVHNode const& node) {
		RigidBody* p_rb = static_cast<RigidBody*>(node.bv.userData);
		SIK_ASSERT(p_rb, "RigidBody was nullptr");
		return function(*p_rb);
	};

	bvh_tree.Query(box, bvQuery);
}
