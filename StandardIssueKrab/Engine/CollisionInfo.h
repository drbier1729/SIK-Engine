#pragma once
//
//#include "RigidBody.h"
//
//// Represents the statement "RigidBody a is colliding with RigidBody b"
//struct Collision
//{
//	RigidBody* a;
//	RigidBody* b;
//};
//
//// Represents a single point of contact between two rigidbodies
//struct Contact
//{
//	Vec3 position;
//	Vec3 normal;
//	Float32 penetration;
//};
//
//// Represents all the contact information between two rigidbodies
//struct ContactBatch 
//{
//	enum {
//		CB_FLAGS_INVERT_NORMAL = 0,
//		CB_FLAGS_REMOVE = 1,
//		CB_FLAGS_COUNT
//	};
//
//	struct Key {
//		RigidBody* a;
//		RigidBody* b;
//	
//		explicit Key(RigidBody* a_, RigidBody* b_)
//			: a{ a_ < b_ ? a_ : b_ },
//			b{ a_ < b_ ? b_ : a_ }
//		{}
//	};
//
//	static constexpr Uint8 MAX_CONTACTS_PER_BATCH = 8;
//	static constexpr SizeT MAX_CONTACT_BATCHES = 2048;
//
//	RigidBody* a;
//	RigidBody* b;
//
//	Float32 friction;
//	Float32 restitution;
//
//	Contact contacts[MAX_CONTACTS_PER_BATCH];
//	Uint8 num_contacts;
//
//	Bitset<CB_FLAGS_COUNT> flags;
//
//	/*
//	* Note: this requires each RigidBody object to have a non-null ptr to 
//	* CollisionProperties... which makes sense because objects without 
//	* CollisionProperties do not have physics-based collision resolution anyway.
//	*/
//	explicit ContactBatch(RigidBody* a, RigidBody* b);
//
//	inline void AddContact(Vec3 const& position, Vec3 const& normal, Float32 penetration);	
//	inline void MarkForDeletion() noexcept;
//	inline Bool ShouldDelete() const noexcept;
//};
//
//
//// Inline definitions
//
//inline void ContactBatch::AddContact(Vec3 const& position, Vec3 const& normal, Float32 penetration) {
//	SIK_ASSERT(num_contacts < MAX_CONTACTS_PER_BATCH, "Limit of contacts reached.");
//	contacts[num_contacts++] = Contact{
//									position,
//									flags.test(CB_FLAGS_INVERT_NORMAL) ? -1.0f * normal : normal,
//									penetration };
//}
//
//inline void ContactBatch::MarkForDeletion() noexcept {
//	a->SetFlag(RigidBody::RB_FLAG_CONTACT_CACHE_IS_DIRTY, true);
//	b->SetFlag(RigidBody::RB_FLAG_CONTACT_CACHE_IS_DIRTY, true);
//	flags.set(CB_FLAGS_REMOVE);
//}
//
//inline Bool ContactBatch::ShouldDelete() const noexcept {
//	return flags.test(CB_FLAGS_REMOVE);
//}
//
//// Various comparison operators for storing and looking up ContactBatchs in
//// sorted containers
//inline bool operator==(ContactBatch const& L, ContactBatch const& R) noexcept {
//	return L.a == R.a && L.b == R.b;
//}
//
//
//inline bool operator<(ContactBatch const& L, ContactBatch const& R) noexcept {
//	return (L.a == R.a) ? L.b < R.b : L.a < R.a; 
//}
//
//inline bool operator<(ContactBatch::Key const& L, ContactBatch::Key const& R) noexcept {
//	return (L.a == R.a) ? L.b < R.b : L.a < R.a;
//}
//
//
//inline bool operator<(ContactBatch::Key const& L, ContactBatch const& R) noexcept {
//	return (L.a == R.a) ? L.b < R.b : L.a < R.a;
//}
//
//inline bool operator<(ContactBatch const& L, ContactBatch::Key const& R) noexcept {
//	return (L.a == R.a) ? L.b < R.b : L.a < R.a;
//}
//
//
//inline bool operator<(RigidBody* L, ContactBatch const& R) noexcept {
//	return L < R.a; 
//}
//
//inline bool operator<(ContactBatch const& L, RigidBody* R) noexcept {
//	return L.a < R; 
//}
//
//inline bool operator<(ContactBatch::Key const& L, RigidBody* R) noexcept {
//	return L.a < R; 
//}
//
//inline bool operator<(RigidBody* L, ContactBatch::Key const& R) noexcept { 
//	return L < R.a;
//}