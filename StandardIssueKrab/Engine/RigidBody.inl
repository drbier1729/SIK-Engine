
inline bool ColliderPair::operator<(ColliderPair const& other) const {
	if (a < other.a) { return true; }
	if (a == other.a && b < other.b) { return true; }
	return false;
}
inline bool ColliderPair::operator==(ColliderPair const& other) const {
	return a == other.a && b == other.b;
}


inline void RigidBody::Enable(Bool val) {
	if (val) { info.set(IS_AWAKE); }
	else	 { info.reset(IS_AWAKE); }
}

inline void RigidBody::MakeTrigger(Bool val) {
	if (val) { info.set(IS_TRIGGER); }
	else	 { info.reset(IS_TRIGGER); }
}

inline void RigidBody::UseBoundingBoxAsCollider(Bool val) {
	if (val) { info.set(USE_AABB_AS_COLLIDER); }
	else	 { info.reset(USE_AABB_AS_COLLIDER); }
}

inline void RigidBody::MakeInvalid() {
	info.set(IS_INVALID);
}

inline Mat4	RigidBody::TransformMatrix() const {
	return glm::translate(Mat4(1), position) * glm::toMat4(orientation);
}

inline Vec3	RigidBody::LocalToWorld(Vec3 const& local_pt) const {
	return orientation * local_pt + position;
}

inline Vec3	RigidBody::WorldToLocal(Vec3 const& world_pt) const {
	return glm::inverse(orientation) * (world_pt - position);
}

inline Vec3	RigidBody::LocalToWorldVec(Vec3 const& local_v) const {
	return orientation * local_v;
}

inline Vec3	RigidBody::WorldToLocalVec(Vec3 const& world_v) const {
	return glm::inverse(orientation) * world_v;
}

inline Bool RigidBody::IsStatic() const {
	return motion_type == MotionType::Static;
}

inline Bool RigidBody::IsDynamic() const {
	return motion_type == MotionType::Dynamic;
}

inline Bool RigidBody::IsKinematic() const {
	return motion_type == MotionType::Kinematic;
}

inline Bool RigidBody::IsValid() const {
	return not info.test(IS_INVALID);
}

inline Bool RigidBody::IsEnabled() const {
	return info.test(IS_AWAKE);
}

inline Bool RigidBody::IsTrigger() const {
	return info.test(IS_TRIGGER);
}

inline Bool RigidBody::IsBoundingBoxUsedAsCollider() const {
	return info.test(USE_AABB_AS_COLLIDER);
}

inline RigidBody::MotionType RigidBody::GetMotionType() const {
	return motion_type;
}