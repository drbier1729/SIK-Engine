#pragma once

struct RigidBody;

namespace Collision {

	struct Contact {
		Vec3    position{};	 // in world coordinates
		Vec3	ra{}, rb{};	 // position of contact point on each body
		Float32 penetration = std::numeric_limits<Float32>::lowest(); // greater than 0 indicates overlap

		//// Info for persistent contacts with sequential impulses
		Float32 impulse_n = 0.0f, impulse_t = 0.0f;
		Float32 mass_n = 0.0f, mass_t = 0.0f;
		Float32 bias = 0.0f;

		// Info for persistent contacts with XPBD
		//Float32 lambda_n = 0.0f, lambda_t = 0.0f;
	};

	struct ContactManifold {
		static constexpr Uint32 MAX_CONTACTS = 8u;

		Contact contacts[MAX_CONTACTS] = {};
		Uint32  num_contacts = 0;
		Vec3    normal = {};  // for objects a and b involved in the collision, this points from a to b
	};


	struct LineSegment {
		Vec3 start = Vec3(0);
		Vec3 end = Vec3(1, 0, 0);

		inline Vec3 Midpoint() const;
		inline LineSegment Transform(Mat4 const& transform) const; // scaling is okay here
	};


	struct AABB final {
		Vec3 position;
		Vec3 halfwidths;

		enum class Face
		{
			Right = 0,
			Top = 1,
			Front = 2,
			Left = 3,
			Bottom = 4,
			Back = 5,
			COUNT
		};

		// Manipulators
		inline AABB& SetMinMax(Vec3 const& min, Vec3 const& max);

		// Queries
		inline Bool Intersects(AABB const& other) const;
		inline Bool Contains(AABB const& other) const;

		ContactManifold Collide(AABB const& other) const;

		ContactManifold CollideAsOBB(Quat const& orientation, AABB const& other, Quat const& other_orientation) const;

		inline Float32 DistSquaredFromPoint(Vec3 const& pt) const;

		// Accessors
		inline Vec3 Min() const;
		inline Vec3 Max() const;
		inline Float32 SurfaceArea() const;

		// Return a modified copy
		inline AABB Transformed(Mat4 const& transform) const; // no scaling. careful: do not apply this repeatedly or AABB will grow infintely.
		inline AABB Transformed(Mat3 const& orientation, Vec3 const& pos) const;
		inline AABB MovedBy(Vec3 const& displacement) const;
		inline AABB Expanded(Float32 const scaleFactor) const;
		inline AABB Union(AABB const& other) const;

		// Niche-use case accessors
		Array<Vec3, 4> FaceAsPolygon(Face face) const;
		LineSegment EdgeAsSegment(Face adjFace1, Face adjFace2) const;
		Vec3 FaceNormal(Face face) const;
		Vec3 SupportingVertex(Vec3 const& localDir) const;
		Face SupportingFace(Vec3 const& localDir) const;
		LineSegment SupportingEdgeWithDirection(Int32 axisIdx, Vec3 const& localDir) const;
		LineSegment SupportingEdge(Vec3 const& localDir) const;

		// Debug only		
		void DebugDraw(GLuint shader, Quat const& rot = Quat(1, 0, 0, 0)) const;
	};


	struct Plane final {
		// dot(normal, p) = d is true for all points p on the plane
		Vec3 normal = Vec3(1, 0, 0);
		Float32 d = 0.0f;

		inline Plane Transform(Mat4 const& transform) const; // scaling has no effect on planes
		inline static Plane Make(Vec3 const& normal, Vec3 const& pt);
	};

	struct Ray
	{
		struct CastResult
		{
			Vec3    point = Vec3(std::numeric_limits<Float32>::max());
			Float32 distance = std::numeric_limits<Float32>::max();
			Bool	hit = false;
		};

		Vec3 p = Vec3(0),		// origin point
			 d = Vec3(1, 0, 0); // (normalized) direction

		inline CastResult Cast(AABB const& aabb) const;
	};

	class Collider {
	public:
		friend class PhysicsManager;

		enum class Type : Uint32 {
			NONE = 0,
			Sphere = 1,
			Capsule = 2,
			Hull = 3
		};

	protected:
		RigidBody const* owner;
		Mat4 local_to_body;
		Mat4 local_to_world;
		Mat3 inertia_local;
		Type type;

	public:
		Float32 mass;
		
	public:
		Collider(Type t);
		virtual ~Collider() noexcept = default;

		virtual AABB GetBoundingBox() const = 0;

		void SetOwner(RigidBody const* owner);
		void UpdateWorldTransform();
		inline void UpdateWorldTransformFromBody(Mat4 const& body_tr);
		
		inline Type GetType() const;
		inline Uint32 GetTypeIdx() const;
		
		inline Vec3 LocalToWorld(Vec3 const& pt) const;
		inline Vec3 LocalToWorldVec(Vec3 const& vec) const;
		inline Vec3 WorldToLocal(Vec3 const& pt) const;
		inline Vec3 WorldToLocalVec(Vec3 const& vec) const;
		inline Mat4 GetLocalToWorldTransform() const;
		inline Mat4 GetWorldRotationMat4() const;
		inline Mat3 GetWorldRotationMat3() const;
		inline Vec3 GetWorldPosition() const;

		inline Vec3 LocalToBody(Vec3 const& pt) const;
		inline Vec3 LocalToBodyVec(Vec3 const& vec) const;
		inline Vec3 BodyToLocal(Vec3 const& pt) const;
		inline Vec3 BodyToLocalVec(Vec3 const& vec) const;
		inline Mat4 GetLocalToBodyTransform() const;
		inline Mat4 GetRelativeRotationMat4() const;
		inline Mat3 GetRelativeRotationMat3() const;
		inline Vec3 GetRelativePosition() const;

		inline void SetLocalToBodyTransform(Mat4 const& tr);
		inline void SetRelativeRotation(Mat3 const& rot);
		inline void SetRelativePosition(Vec3 const& pos);

		inline Mat3 GetInertiaTensorInSpace(Vec3 const& translation, Mat3 const& rotation) const;
		inline Mat3 GetInertiaTensorRelative() const; // Inertia tensor about owner's COM
		inline Mat3 GetInertiaTensorWorldRotation() const;

		ContactManifold Collide(Collider const* other) const;
		Bool			BoundsIntersect(Collider const* other) const;
	};

	
	class Sphere final : public Collider {
		Float32 radius;
	
	public:
		explicit Sphere(Float32 radius = 1.0f);
		~Sphere() noexcept = default;
		
		inline Float32 GetRadius() const;
		inline void SetRadius(Float32 r);

		AABB GetBoundingBox() const override;

	private:
		inline void ComputeInertiaTensor();
	};
	
	class Capsule final : public Collider {
		Float32 seg_halflength;
		Float32 radius;

	public:
		Capsule();
		Capsule(Float32 radius, Float32 segment_length);
		~Capsule() noexcept = default;

		inline LineSegment GetCentralSegment() const;

		inline Float32 GetLength() const;
		inline Float32 GetRadius() const;

		inline void SetLength(Float32 length);
		inline void SetRadius(Float32 radius);

		AABB GetBoundingBox() const override;

	private:
		void ComputeInertiaTensor();
	};
	

	// Assumes this has centroid/center of mass at the local origin
	class Hull final : public Collider {
	public:
		static constexpr Uint8 MAX_EDGES = std::numeric_limits<Uint8>::max();

		struct HalfEdge {
			Uint8 next = 0;
			Uint8 twin = 0;
			Uint8 origin = 0;
			Uint8 face = 0;;
		};

		struct Face {
			Uint8 edge = 0;
		};

	public:
		Vec3             bounds;   // in local space
		Vector<Vec3>     vertices; // in local space
		Vector<HalfEdge> edges;	   // stored s.t. each edge is adjacent to its twin
		Vector<Face>     faces;
		Vector<Plane>    planes;

	public:
		Hull();
		~Hull() noexcept = default;

		AABB			GetBoundingBox() const override;

		// Must be called after modifying vertices field
		void			ComputeInertiaTensor();

		// These scale the vertices locally, then recompute the inertia tensor. They
		// are fairly expensive operations and should be used sparingly, if at all.
		inline void		Scale(Vec3 const& scale);
		inline void		Scale(Float32 x, Float32 y, Float32 z);


		// Since boxes are very common, let's just make it
		// easy to create them
		static Hull		BoxInstance(Vec3 const& halfwidths);


		// Returns point in local space
		inline Vec3 GetSupport(Vec3 const& direction_local) const;
	};
}

struct SphereArgs {
	Float32 radius;
};

struct CapsuleArgs {
	Float32 length;
	Float32 radius;
};

struct HullArgs {
	Vec3 halfwidths;
	Bool is_box;
};

// Use this to create colliders in the PhysicsManager
struct ColliderCreationSettings {
	using Type = Collision::Collider::Type;
	
	Type type = Type::NONE;
	
	Vec3 position_offset    = Vec3(0);
	Quat orientation_offset = Quat(1, 0, 0, 0);
	Float32 mass			= 0.0f;

	union {
		Uint64 NO_ARGS = 0ull;
		SphereArgs sphere_args;
		CapsuleArgs capsule_args;
		HullArgs hull_args;
	};
};


#include "Collision.inl"