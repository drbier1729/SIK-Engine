#pragma once

#include "Collision.h"

class GameObject;
struct RigidBody;
struct MotionProperties;


struct ColliderPair {
	ColliderPair(RigidBody* a_, Uint32 idx_a_, RigidBody* b_, Uint32 idx_b_);

	RigidBody* a, *b;
	Uint32 idx_a, idx_b;

	inline bool operator<(ColliderPair const& other) const;
	inline bool operator==(ColliderPair const& other) const;
};

struct alignas(16) RigidBody {

public:
	ALLOW_PRIVATE_REFLECTION

	friend class PhysicsManager;
	friend struct CollisionArbiter;
	friend class Factory;

	enum class MotionType {
		Static,
		Dynamic,
		Kinematic
	};

	enum Info {
		IS_INVALID = 0,
		IS_AWAKE = 1,
		IS_TRIGGER = 2,
		USE_AABB_AS_COLLIDER = 3,
		COUNT
	};

public:
	static const RigidBody world_body;
	static constexpr Uint32 MAX_COLLIDERS = 6;
	using CollidersList = Collision::Collider* [MAX_COLLIDERS];

	// Connection to the Game World
	GameObject* owner = nullptr;

	// Flags
	MotionType           motion_type = MotionType::Static;
	Bitset<Info::COUNT>  info = 0;

	// Dynamics (suffix L indicates local coords)
	Vec3				 position = Vec3(0); // world coords of centroid/center of mass
	Quat                 orientation = Quat(1, 0, 0, 0);
	Vec3				 centroid_L = Vec3(0);
	MotionProperties*    motion_props = nullptr;

	// Collision Detection + Resolution
	CollidersList		 colliders = {};
	Collision::AABB		 bounds = {};
	Collision::AABB		 local_bounds = {}; // TODO: remove this and replace with GetBoundingBox() method
	Uint32				 num_colliders = 0;
	Float32				 friction = 0.0f;     // 0 --> frictionless
	Float32				 restitution = 1.0f;  // this does not work yet! 0 --> fully inelastic, 1 --> fully elastic


public:
	// Manipulators
	void			  AddForce(Vec3 const& force);
	void			  AddForceAtWorldPoint(Vec3 const& force, Vec3 const& world_pos);
	void			  AddForceAtLocalPoint(Vec3 const& force, Vec3 const& local_pos);
	void			  UpdateCentroidFromOwnerPosition();
	void			  SyncWithOwnerTransform(Float32 dt, bool interpolate = true);
	inline void		  Enable(Bool val = true);
	inline void		  MakeTrigger(Bool val = true);
	inline void		  MakeInvalid();
	inline void		  UseBoundingBoxAsCollider(Bool val = true);

	// Accessors
	inline Bool       IsStatic() const;
	inline Bool       IsDynamic() const;
	inline Bool       IsKinematic() const;
	inline Bool       IsValid() const;
	inline Bool       IsEnabled() const;
	inline Bool       IsTrigger() const;
	inline Bool		  IsBoundingBoxUsedAsCollider() const;
	inline MotionType GetMotionType() const;
	// inline AABB       GetBoundingBox() const; // applies transform to bounds

	inline Mat4		  TransformMatrix() const;
	inline Vec3		  LocalToWorld(Vec3 const& local_pt) const;
	inline Vec3		  WorldToLocal(Vec3 const& world_pt) const;
	inline Vec3		  LocalToWorldVec(Vec3 const& local_v) const;
	inline Vec3		  WorldToLocalVec(Vec3 const& world_v) const;

	// Recomputes AABB halfwidths based on colliders' bounds
	void UpdateAABB();
	
	

private:
	// Called once RigidBody is completely created, i.e., it has 
	// all its colliders and its motion properties set. Computes
	// the local inertia tensor and total mass.
	void ComputeConstants();

	// Adds colliders and computes their local-->world transforms based on
	// offsets from the RigidBody's centroid and orientation
	void AddColliders(Collision::Collider* const colliders[], Uint32 num_colliders);

	// Adds motion properties and initializes their previous centroid
	// and orientation to the current position and orientation
	void AddMotionProperties(MotionProperties* mp);

	// Recomputes colliders' local to world transform matrices, and AABB position
	void UpdateInternals();

	// Tests bounding box/volume intersections with other RB
	Vector<ColliderPair> Intersect(RigidBody* other);

	// Sequential Impulses
	void IntegrateForces(Float32 time_step);
	void IntegrateVelocities(Float32 time_step);


	// // OLD XPBD
	//void UpdatePositions(Float32 time_step);
	//void UpdateVelocities(Float32 time_step);
};


// Creating RigidBodies through the PhysicsManager can be done by using RigidBodyCreationSettings

struct RigidBodyCreationSettings {
	Vec3					  position = Vec3(0);
	Quat					  orientation = Quat(1, 0, 0, 0);

	ColliderCreationSettings  collider_parameters[RigidBody::MAX_COLLIDERS] = {};
	Vec3					  aabb_halfwidths = Vec3(1);
	
	RigidBody::MotionType     motion_type = RigidBody::MotionType::Static;
	Float32					  gravity_scale = 0.0f;
	Float32					  linear_damping = 0.0f;
	Float32					  angular_damping = 0.0f;

	Float32					  mass = 0.0f;
	Float32					  friction = 0.0f;
	Float32					  restitution = 0.0f;
	
	Bool					  is_enabled = true;
	Bool					  is_trigger = false;
	Bool					  use_aabb_as_collider = false;
};

#include "RigidBody.inl"