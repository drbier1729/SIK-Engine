#include "stdafx.h"
#include "PhysicsManager.h"

#include "GameObject.h"	// used in Extrapolate and collision callbacks
#include "Transform.h"
#include "CollisionDebugDrawing.h"
#include "Mesh.h"

PhysicsManager::PhysicsManager()
	: rigidbodies{},
	dynamic_bodies{},
	motion_properties{},
	colliders{},
	bvh_tree{},
	broad_phase_results{},
	arbiters{},
	debug_wireframes{},
	cn_plane_mesh{ Collision::WireframeMesh(Collision::Plane{
		.normal = Vec3(1,0,0),
		.d = 1
		}) }
{}


////////////////////////////////////////////////////////////////////////////
// MANIPULATORS
////////////////////////////////////////////////////////////////////////////


RigidBody* PhysicsManager::CreateRigidBody(RigidBodyCreationSettings const& rb_settings) {
	using Collision::Collider;

	RigidBody& rb = *rigidbodies.insert();

	// Simple fields
	rb.position = rb_settings.position;
	rb.orientation = rb_settings.orientation;

	rb.local_bounds.halfwidths = rb_settings.aabb_halfwidths;
	rb.local_bounds.position = Vec3(0);

	rb.friction = rb_settings.friction;
	rb.restitution = rb_settings.restitution;
	
	// Flags
	rb.Enable(rb_settings.is_enabled);
	rb.MakeTrigger(rb_settings.is_trigger);
	rb.UseBoundingBoxAsCollider(rb_settings.use_aabb_as_collider);

	// Colliders
	Collider* cols[RigidBody::MAX_COLLIDERS] = {};
	Uint32 count = 0u;
	for (; count < RigidBody::MAX_COLLIDERS; ++count) {

		ColliderCreationSettings const& col_settings = rb_settings.collider_parameters[count];
		if (col_settings.type == Collider::Type::NONE) { break; }

		cols[count] = colliders.Add(col_settings, bvh_tree);
	}
	rb.AddColliders(cols, count);

	// Collider debug drawing
	if (rb.num_colliders > 0) {
		debug_wireframes.push_back(std::make_pair(&rb, Vector<Collision::WireframeMesh>{}));
		auto& rb_wireframes = debug_wireframes.back().second;

		for (auto i = 0u; i < rb.num_colliders; ++i) {
			Collider* c = rb.colliders[i];
			Collision::WireframeMesh w{};
			switch (c->GetType())
			{
			break; case Collider::Type::Sphere: {
				w = Collision::WireframeMesh{ *static_cast<Collision::Sphere*>(c) };
			}
			break; case Collider::Type::Capsule: {
				w = Collision::WireframeMesh{ *static_cast<Collision::Capsule*>(c) };
			}
			break; case Collider::Type::Hull: {
				w = Collision::WireframeMesh{ *static_cast<Collision::Hull*>(c) };
			}
			}
			w.FinalizeLines();
			rb_wireframes.push_back(std::move(w));
		}
	}

	// Motion propertties
	rb.motion_type = rb_settings.motion_type;
	if (rb.motion_type != RigidBody::MotionType::Static) {
		rb.AddMotionProperties(motion_properties.insert());
		rb.motion_props->gravity_scale = rb_settings.gravity_scale;
		rb.motion_props->linear_damping = rb_settings.linear_damping;
		rb.motion_props->angular_damping = rb_settings.angular_damping;
	}

	rb.ComputeConstants();
	if (rb.motion_props && rb.motion_props->mass == 0.0f) {
		rb.motion_props->SetMass(rb_settings.mass);
	}

	// Add to broad phase BVH
	Collision::BVHandle handle = bvh_tree.Insert(rb.bounds, &rb);
	bv_handles.insert_or_assign(&rb, handle);
	bvh_tree.SetMoved(handle, false);
	moved_last_frame.push_back({ &rb, handle });

	if (rb.IsDynamic()) {
		dynamic_bodies.push_back(&rb);
	}

	return &rb;
}

void PhysicsManager::RemoveRigidBody(RigidBody* rb) {
	rb->info.set(RigidBody::IS_INVALID);
}

RigidBody* PhysicsManager::RemoveRigidBodyInternal(RigidBody* rb) {

	if (rb->IsDynamic()) {
		for (auto&& p : dynamic_bodies) {
			if (p == rb) {
				p = dynamic_bodies.back();
				dynamic_bodies.pop_back();
				break;
			}
		}
	}

	if (rb->motion_props != nullptr) {
		motion_properties.erase(rb->motion_props);
	}

	for (auto i = 0u; i < rb->num_colliders; ++i) {
		// NOTE THAT CONTACT MANIFOLDS ARE NOT CLEANED UP HERE
		colliders.Remove(rb->colliders[i]);
	}

	// Remove from broad phase
	auto it = bv_handles.find(rb);
	if (it != bv_handles.end()) {
		bvh_tree.Remove(it->second);
		bv_handles.erase(it);
	}

	if (rb->owner != nullptr) {
		rb->owner->rigidbody = nullptr;
	}

	return rigidbodies.erase(rb);
}


RayCastHit PhysicsManager::RayCast(Collision::Ray const& ray)
{
	// O(n) brute force -- can be simplified with broadphase structure to O(logn)
	RayCastHit result{};

	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		
		RigidBody& rb = r.front();

		if (auto cast = ray.Cast(rb.bounds); cast.hit) {
			if (rb.IsBoundingBoxUsedAsCollider() && cast.distance < result.info.distance) {
				result.info = cast;
				result.object = rb.owner;
			}
			else {
				for (Uint32 i = 0; i < rb.num_colliders; ++i) {
					Collision::Collider* c = rb.colliders[i];
					cast = ray.Cast(c->GetBoundingBox());
					if (cast.distance < result.info.distance) {
						result.info = cast;
						result.object = rb.owner;
					}
				}
			}
		}
	}
	

	if (result.object == nullptr) {
		static Collision::AABB ground_plane{ .position = Vec3(0, 0, 0), .halfwidths = {1000, 0, 1000} };
		auto cast = ray.Cast(ground_plane);
		result.info = cast;
	}

	return result;
}

void PhysicsManager::Update(Float32 time_step) {
	using namespace Collision;
	
	static constexpr Uint32 numSubsteps = 2;

	Float32 h = time_step / numSubsteps;

	RemoveTombstoned();

	DetectCollisionsBroad_v2();

	DetectCollisionsNarrow_v2();

	// Sim substep loop
	for (Uint32 i = 0; i < numSubsteps; ++i) {

		for (auto&& dyn : dynamic_bodies) {
			if (dyn->IsEnabled()) {
				dyn->IntegrateForces(h);
				dyn->UpdateInternals();
			}
		}

		if (collisions_active) {
			for (auto&& [key, arb] : arbiters) {
				arb.PreStep(h);
			}
			// PreStep for constraints
			// ... joints, etc

			for (auto i = 0; i < 10; ++i) {
				for (auto&& [key, arb] : arbiters) {
					arb.ApplyImpulse();
				}
			}
		}

		// Solve constraints
		SolveGroundConstraint();
		// ... joints, etc

		for (auto&& dyn : dynamic_bodies) {
			if (dyn->IsEnabled()) {
				dyn->IntegrateVelocities(h);
			}
		}
	}

	// Collision callbacks - note this might miss some very fast (< 1 frame) collisions
	// Do we guarantee that all rigidbodies here are valid? I believe so...
	// Note since this is iterating through ColliderPairs it will call OnCollide
	// once pair of colliding colliders attached to the rigidbodies
	if (collisions_active) {
		for (auto&& [p, a] : arbiters) {
			if (a.manifold.num_contacts > 0) {
				SIK_ASSERT(p.a->owner && p.b->owner, "Owning GameObjects must be valid here.");
				p.a->owner->OnCollide(p.b->owner);
				p.b->owner->OnCollide(p.a->owner);
			}
		}
	}
}

// Checks for overlapping bounding volumes O(n^2)
void PhysicsManager::DetectCollisionsBroad() noexcept {
	
	for (auto&& dyn : dynamic_bodies) {
		dyn->UpdateAABB();
	}

	broad_phase_results.clear();

	for (auto r_a = rigidbodies.all(); not r_a.is_empty(); r_a.pop_front()) {

		RigidBody& a = r_a.front();
		if (not a.IsEnabled()) { continue; }

		RigidBody::MotionType mt_a = a.GetMotionType();

		auto r_b = r_a;
		r_b.pop_front();

		for (; not r_b.is_empty(); r_b.pop_front()) {

			RigidBody& b = r_b.front();
			if (not b.IsEnabled()) { continue; }

			RigidBody::MotionType mt_b = b.GetMotionType();
			if (mt_a == RigidBody::MotionType::Static &&
				mt_b == RigidBody::MotionType::Static) {
				continue;
			}

			if (a.IsBoundingBoxUsedAsCollider() || b.IsBoundingBoxUsedAsCollider()) {
				if (a.bounds.Intersects(b.bounds)) {
					broad_phase_results.push_back(ColliderPair{ 
							&a, RigidBody::MAX_COLLIDERS, 
							&b, RigidBody::MAX_COLLIDERS });
				}
				else {
					auto it = arbiters.equal_range(ColliderPair{ &a, 0, &b, 0 });
					arbiters.erase(it.first, it.second);
				}
			}
			else {
				auto potential_collisions = a.Intersect(&b);
				for (auto&& p : potential_collisions) {
					broad_phase_results.push_back(p);
				}
				if (potential_collisions.empty()) {
					auto it = arbiters.equal_range(ColliderPair{ &a, 0, &b, 0 });
					arbiters.erase(it.first, it.second);
				}
			}
		}
	}
}


void PhysicsManager::DetectCollisionsBroad_v2() noexcept
{
	using namespace Collision;

	// Update AABBs and the BVH -- do this for all bodies since 
	// static bodies might have been moved by the user
	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {

		RigidBody& rb = r.front();
		if (not rb.IsEnabled()) { continue; }

		AABB const preBounds = rb.bounds;
		rb.UpdateAABB();

		// Update broad phase
		auto it = bv_handles.find(&rb);
		if (it == bv_handles.end()) {
			Collision::BVHandle newHandle = bvh_tree.Insert(rb.bounds, &rb);
			bv_handles.insert_or_assign(&rb, newHandle);
			bvh_tree.SetMoved(newHandle, false);
			moved_last_frame.push_back({ &rb, newHandle });
		}
		else {
			// Compute swept bounding box from prev time step to now    
			AABB const projectedBounds = rb.bounds.Union(preBounds);

			Vec3 const disp = rb.bounds.position - preBounds.position;
			BVHandle const handle = it->second;
			Bool const moved = bvh_tree.MoveBoundingVolume(handle, projectedBounds, disp);
			if (moved) {
				moved_last_frame.push_back({ &rb, handle });
			}
			else {
				bvh_tree.SetMoved(handle, false);
			}
		}
	}

	// Collect pairs of potentially colliding RigidBodies
	for (auto&& [query_rb, query_handle] : moved_last_frame) {

		BVHNode const* query_node = bvh_tree.Find(query_handle);
		if (not query_node) { continue; }

		BV const& bv = query_node->bv;

		bvh_tree.Query(bv.fatBounds, [this, query_handle, query_node, query_rb](BVHNode const& node) -> Bool {

			if (query_node->index == node.index)
			{
				// Avoid collisions with self
				return true;
			}
			if (node.moved && node.index > query_node->index)
			{
				// Both objects are moving -- avoid duplicate collisions
				return true;
			}

			RigidBody* rb = static_cast<RigidBody*>(node.bv.userData);
			if (rb == query_rb || not rb->IsEnabled())
			{
				// Avoid collisions between different parts of the same RigidBody
				return true;
			}

			broad_phase_results.push_back(ColliderPair{rb, 0, query_rb, 0});
			return true;
		});
	}

	// Reset moved flags on bounding volumes -- TODO: is this loop necessary here?
	for (auto&& [rb, handle] : moved_last_frame)
	{
		bvh_tree.SetMoved(handle, false);
	}
	moved_last_frame.clear();
}


void PhysicsManager::DetectCollisionsNarrow() noexcept {
	for (auto&& p : broad_phase_results) {
		
		CollisionArbiter new_arb{ p };

		if (new_arb.manifold.num_contacts > 0) {
			// If an arbiter already exists, update it
			if (auto it = arbiters.find(p); it != arbiters.end()) {
				it->second.Update(new_arb.manifold);
			}
			// If we found a new set of contacts, add the arbiter to our cache
			else {
				arbiters.insert({ p, new_arb });
			}
		}

		// No contacts, so erase the arbiter
		else {
			arbiters.erase(p);
		}
	}
}

void PhysicsManager::DetectCollisionsNarrow_v2() noexcept {
	using namespace Collision;

	// Remove existing arbiters which fail to collide in broad phase,
	// and update the ones which do
	for (auto it = arbiters.begin(); it != arbiters.end();) {
		// Decompose into key (ColliderPair) and value (Arbiter)
		auto& [p, a] = *it;

		BVHandle const handleA = bv_handles.at(p.a);
		BVHandle const handleB = bv_handles.at(p.b);

		// Midphase pre-check: recheck fat AABBs for overlap
		AABB const fatBoundsA = bvh_tree.Find(handleA)->bv.fatBounds;
		AABB const fatBoundsB = bvh_tree.Find(handleB)->bv.fatBounds;
		if (not fatBoundsA.Intersects(fatBoundsB)) {
			// If no overlap, remove the contact manifold
			it = arbiters.erase(it);
		}
		else {
			// Update the arbiter with narrow phase collision detection
			CollisionArbiter const new_arb{ p };
			a.Update(new_arb.manifold);
			++it;
		}
	}

	// Check broad phase pairs and do narrow phase
	for (auto&& p : broad_phase_results) {

		// May not be needed, but let's just double-check this here
		if (p.a->IsStatic() && p.b->IsStatic()) { continue; }

		// If an arbiter does not exist, add it
		auto it = arbiters.find(p);
		if (it == arbiters.end()) {
			CollisionArbiter const new_arb{ p };
			arbiters.insert({ p, new_arb });
		}

		// Else: the arbiter already exists and we updated it
		// already, so do nothing
	}

	// Now that we've added all broad phase results to narrow phase
	// we can clear these so future substeps don't reiterate this
	broad_phase_results.clear();
}


void PhysicsManager::SolveGroundConstraint() noexcept {
	static constexpr Float32 ground_plane_height = -0.9f;

	for (auto&& dyn : dynamic_bodies) {
		
		// AABB version
		Float32 const hh = dyn->local_bounds.halfwidths.y + ground_plane_height;
		if (dyn->position.y < hh) {
			dyn->position.y = hh;
			dyn->motion_props->prev_position.y = hh;
			dyn->motion_props->linear_velocity.y = 0.0f;
		}
	}
}

void PhysicsManager::Clear() noexcept {
	broad_phase_results.clear();
	moved_last_frame.clear();
	bvh_tree.Clear();
	bv_handles.clear();
	arbiters.clear();
	colliders.Clear();
	motion_properties.clear();
	dynamic_bodies.clear();

	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		if (GameObject* owner = r.front().owner; owner != nullptr) {
			owner->rigidbody = nullptr;
		}
	}

	rigidbodies.clear();

	// Debug
	debug_wireframes.clear();
}


void PhysicsManager::RemoveTombstoned() noexcept {

	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		RigidBody& rb = r.front();

		// Clear out of bounds and orphaned rigidbodies
		if (glm::distance2(rb.position, Vec3(0)) > 1.0e9f ||
			not rb.owner) 
		{
			rb.info.set(RigidBody::IS_INVALID);
		}
	}

	// DEBUG
	std::erase_if(debug_wireframes,
		[](auto&& item) { 
			return not item.first->IsValid(); 
		}
	);


	std::erase_if(arbiters,
		[](auto&& item) {
			return	not item.first.a->IsValid() ||
					not item.first.b->IsValid() ||
					not item.first.a->IsEnabled() ||
					not item.first.b->IsEnabled();
		}
	);


	std::erase_if(moved_last_frame, 
		[](auto&& item) {
			RigidBody* rb = std::get<RigidBody*>(item);
			return not rb->IsValid() || not rb->IsEnabled();
		}
	);


	// Remove invalid rigidbodies
	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		RigidBody& rb = r.front();
		if (not rb.IsValid()) {
			RemoveRigidBodyInternal(&rb);
		}
		else if (not rb.IsEnabled()) {
			auto it = bv_handles.find(&rb);
			if (it != bv_handles.end()) {
				bvh_tree.Remove(it->second);
				bv_handles.erase(it);
			}
		}
	}

}


void PhysicsManager::ToggleCollisions() {
	collisions_active = !collisions_active;
	MotionProperties::gravity = Vec3(0, static_cast<Float32>(collisions_active) * -9.8f, 0);
}

////////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////////

void PhysicsManager::Extrapolate(Float32 extrapolation) noexcept {

	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		r.front().SyncWithOwnerTransform(extrapolation, false);
	}

}


void PhysicsManager::Interpolate(Float32 interpolation) noexcept {
	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		r.front().SyncWithOwnerTransform(interpolation, true);
	}
}

Mat3 ArbitraryRotation(Vec3 v, bool IsInverse) {
	
	Vec3 A = glm::normalize(v);
	Vec3 Up(0.0f, 1.0f, 0.0f);	
	Vec3 B = glm::normalize(glm::cross(Up, A));
	if (glm::any(glm::isnan(B))) {
		Up = Vec3(1.0f, 0.0f, 0.0f);
		B = glm::normalize(glm::cross(Up, A));
	}
	Vec3 C = glm::cross(A, B);
	Mat3 Rt(B, C, A);
	Mat3 R = glm::transpose(Rt);

	return IsInverse ? Rt : R;
}


// TODO : remove when we are able to
#include "BoxGeometry.h"
#include "RenderCam.h"


static void DrawContactManifold(Collision::ContactManifold const& manifold,
	Collision::WireframeMesh const& cn_plane_mesh,
	GLuint shader_id) {

	if (manifold.num_contacts == 0) { return; }

	
	Vec4 const cn_color{ 0.5, 1, 0, 1 };
	int loc = glGetUniformLocation(shader_id, "Color");
	glUniform4fv(loc, 1, &cn_color[0]);

	// Draw collision plane
	//Mat4 const tr = glm::translate(manifold.contacts[0].position) * Collision::detail::BasisFromUnitVector(manifold.normal);
	//cn_plane_mesh.Draw(shader_id, tr);

	// Draw contact points
	Vec4 const pt_color{ 0, 1, 0, 1 };
	loc = glGetUniformLocation(shader_id, "Color");
	glUniform4fv(loc, 1, &pt_color[0]);

	// Just use the cube mesh for the contact points...
	Mesh const& cube_mesh = Mesh::Cube();
	cube_mesh.Use();
	for (auto i = 0u; i < manifold.num_contacts; ++i) {
		Mat4 const pt_tr = glm::translate(manifold.contacts[i].position) * glm::scale(Vec3(0.05f));

		loc = glGetUniformLocation(shader_id, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(pt_tr[0][0]));

		cube_mesh.Draw();
	}
	cube_mesh.Unuse();
}

void PhysicsManager::DebugDraw(GLuint shader_id, const RenderCam* cam, Float32 extrapolation, PhysDebugBox const& gfx_box) noexcept {

	//if (not cn_plane_mesh.IsLocked()) { cn_plane_mesh.FinalizeLines(); }
	//if (not h_mesh.IsLocked()) { h_mesh.FinalizeLines(); }
	//if (not h2_mesh.IsLocked()) { h2_mesh.FinalizeLines(); }
	//if (not c_mesh.IsLocked()) { c_mesh.FinalizeLines(); }
	//if (not c2_mesh.IsLocked()) { c2_mesh.FinalizeLines(); }

	Mat4 I = Mat4(1);

	glUseProgram(shader_id);
	int loc = 0;

	Mat4 view = cam->GetViewMat();
	Mat4 proj = cam->GetProjMat();

	loc = glGetUniformLocation(shader_id, "View");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(view[0][0]));

	loc = glGetUniformLocation(shader_id, "Proj");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(proj[0][0]));

	// OBBs
	for (auto r = rigidbodies.all(); not r.is_empty(); r.pop_front()) {
		RigidBody const& rb = r.front();
		if (rb.IsEnabled()) {
			Collision::AABB asOBB = rb.local_bounds;
			asOBB.position += rb.position;
			asOBB.DebugDraw(shader_id, rb.orientation);
		}
	}

	// Broadphase AABBs
	/*bvh_tree.ForEach([&](Collision::BV const& bv) {
		bv.fatBounds.DebugDraw(shader_id);
	});*/

	// Colliders
	for (auto&& [rb, wireframes] : debug_wireframes) {
		SIK_ASSERT(rb->num_colliders == wireframes.size(), "Size mismatch.");
		if (rb->IsEnabled()) {
			for (auto i = 0u; i < rb->num_colliders; ++i) {
				Collision::Collider* c = rb->colliders[i];

				Mat4 tr = c->GetLocalToWorldTransform();
				wireframes[i].Draw(shader_id, tr);

				c->GetBoundingBox().DebugDraw(shader_id);
			}
		}
	}
	
	// Collisions
	for (auto&& [p, a] : arbiters) {
		if (a.manifold.num_contacts > 0) {
			DrawContactManifold(a.manifold, cn_plane_mesh, shader_id);
		}
	}

	// TESTING
	//{
	//	// Hulls
	//	{
	//		Mat4 tr;
	//		Vec4 const color = hull_cn.num_contacts > 0 ? Vec4(1, 0, 0, 1) : Vec4(0, 0, 1, 1);
	//		loc = glGetUniformLocation(shader_id, "Color");
	//		glUniform4fv(loc, 1, &color[0]);

	//		tr = h.GetLocalToWorldTransform();
	//		h_mesh.Draw(shader_id, tr);

	//		tr = h2.GetLocalToWorldTransform();
	//		h2_mesh.Draw(shader_id, tr);
	//	}

	//	// Spheres
	//	{
	//		Mesh const& sph_mesh = Mesh::Sphere();

	//		Vec4 const color = Vec4(0, 0, 1, 1);
	//		loc = glGetUniformLocation(shader_id, "Color");
	//		glUniform4fv(loc, 1, &color[0]);

	//		sph_mesh.Use();

	//		Mat4 tr = s.GetLocalToWorldTransform() * glm::scale(Vec3(s.GetRadius()));
	//		loc = glGetUniformLocation(shader_id, "ModelTr");
	//		glUniformMatrix4fv(loc, 1, GL_FALSE, &(tr[0][0]));
	//		sph_mesh.Draw();

	//		tr = s2.GetLocalToWorldTransform() * glm::scale(Vec3(s2.GetRadius()));
	//		loc = glGetUniformLocation(shader_id, "ModelTr");
	//		glUniformMatrix4fv(loc, 1, GL_FALSE, &(tr[0][0]));
	//		sph_mesh.Draw();

	//		sph_mesh.Unuse();
	//	}

	//	// Capsules
	//	{
	//		Vec4 const color = Vec4(0, 0, 1, 1);
	//		loc = glGetUniformLocation(shader_id, "Color");
	//		glUniform4fv(loc, 1, &color[0]);

	//		Mat4 tr = c.GetLocalToWorldTransform();
	//		c_mesh.Draw(shader_id, tr);

	//		tr = c2.GetLocalToWorldTransform();
	//		c2_mesh.Draw(shader_id, tr);
	//	}
	//}



	//if (hull_cn.num_contacts > 0) {
	//	DrawContactManifold(hull_cn, cn_plane_mesh, shader_id);
	//}
	//if (sph_cn.num_contacts > 0) {
	//	DrawContactManifold(sph_cn, cn_plane_mesh, shader_id);
	//}
	//if (cap_cn.num_contacts > 0) {
	//	DrawContactManifold(cap_cn, cn_plane_mesh, shader_id);
	//}
	//// Capsule-Sphere
	//if (sph_cap_cn.num_contacts > 0) {
	//	DrawContactManifold(sph_cap_cn, cn_plane_mesh, shader_id);
	//}

	glUseProgram(0);
}

void PhysicsManager::DebugDraw_Forces(GLuint shader_id, const RenderCam* cam, Float32 extrapolation, Arrow const& arrow) noexcept {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Mat4 I = Mat4(1);

	glUseProgram(shader_id);
	int loc = 0;

	Mat4 view = cam->GetViewMat();
	Mat4 proj = cam->GetProjMat();

	loc = glGetUniformLocation(shader_id, "View");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(view[0][0]));

	loc = glGetUniformLocation(shader_id, "Proj");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(proj[0][0]));

	for (auto r = rigidbodies.all(); !r.is_empty(); r.pop_front()) {
		RigidBody const& rb = r.front();
		
		if (not rb.IsEnabled())
			continue;

		Mat4 model_tr = I;
		Mat4 rotation_matrix = I;
		Vec3 A(0);
		Mat3 AZ(1), ZB(1), AB(1);
		Float32 m = 0.0f;

		// draw velocity
		if (rb.motion_props == nullptr) continue;
		if (glm::any(glm::isnan(rb.motion_props->linear_velocity))) continue;

		// Calculate rotation matrix (3x3)
		A = Vec3(0.0f, 1.0f, 0.0f);
		AZ = ArbitraryRotation(A, false);
		ZB = ArbitraryRotation(rb.motion_props->linear_velocity, true);
		AB = ZB * AZ;

		// turn 3x3 matrix to 4x4 matrix
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				rotation_matrix[i][j] = AB[i][j];
			}
		}

		// Get magnitude of velocity to scale the arrow
		m = glm::length(rb.motion_props->linear_velocity);
		model_tr = glm::translate(I, rb.position) * rotation_matrix * glm::scale(I, Vec3(1.0, m, 1.0));
		
		Vec4 color(1, 1, 0, 0);
		loc = glGetUniformLocation(shader_id, "Color");
		glUniform4fv(loc, 1, &color[0]);
		loc = glGetUniformLocation(shader_id, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(model_tr[0][0]));
		
		arrow.DrawLine();

		// draw accumulated force
		if (glm::any(glm::isnan(rb.motion_props->accumulated_force))) continue;

		ZB = ArbitraryRotation(rb.motion_props->accumulated_force, true);
		AB = ZB * AZ;

		// turn 3x3 matrix to 4x5 matrix
		rotation_matrix = I;
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				rotation_matrix[i][j] = AB[i][j];
			}
		}

		model_tr = glm::translate(I, rb.position) * rotation_matrix * glm::scale(I, Vec3(1.0, 1.0, 1.0));
		color = Vec4(1, 1, 1, 0);
		loc = glGetUniformLocation(shader_id, "Color");
		glUniform4fv(loc, 1, &color[0]);
		loc = glGetUniformLocation(shader_id, "ModelTr");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &(model_tr[0][0]));
		arrow.DrawLine();
	}

	glUseProgram(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



#pragma region OLD XPBD STUFF

//void PhysicsManager::Update(FrameTimer::duration_type& accumulated_time, FrameTimer::duration_type dt, Uint32 integration_substeps) {
//
//	Float32 h = FrameTimer::ToSeconds<Float32>(dt);
//	
//	RemoveTombstoned();
//
//	// Broad phase
//	for (auto&& dyn : dynamic_bodies) {
//		dyn->UpdateAABB();
//	}
//	DetectCollisionsBroad();
//
//	// TESTING-----------------------
//	// sphere-sphere
//	sph_cn = s.Collide(&s2);
//
//	// sphere-capsule
//	sph_cap_cn = s.Collide(&c);
//	
//	// capsule-capsule
//	cap_cn = c.Collide(&c2);
//
//	// hull-hull
//	hull_cn = (this->h).Collide(&h2);
//	// END TESTING-------------------
//
//	// Substep loop
//	while (accumulated_time >= dt) {
//
//		for (auto&& dyn : dynamic_bodies) {
//			dyn->UpdatePositions(h);
//			dyn->UpdateInternals();
//		}
//
//		DetectCollisionsNarrow();
//		XPBDSolvePositions();
//		
//		for (auto&& dyn : dynamic_bodies) {
//			dyn->UpdateVelocities(h);
//		}
//		XPBDSolveVelocities(h);
//		
//		accumulated_time -= dt;
//	}
//
//	// Update game objects
//	Extrapolate(FrameTimer::ToSeconds<Float32>(accumulated_time));
//
//	// Collision callbacks
//	for (auto&& [p, m] : narrow_phase_results) {
//		if (p.a && p.b && p.a->owner && p.b->owner) {
//			if (m.num_contacts > 0) {
//				p.a->owner->OnCollide(p.b->owner);
//				p.b->owner->OnCollide(p.a->owner);
//			}
//		}
//	}
//}
//
//void PhysicsManager::XPBDSolvePositions() {
//
//	// Solve Collisions
//	for (auto&& [key, manifold] : narrow_phase_results) {
//		if (manifold.num_contacts == 0) { continue; }
//
//		RigidBody& a = *key.a;
//		RigidBody& b = *key.b;
//
//		MotionProperties* mp_a = a.motion_props;
//		MotionProperties* mp_b = b.motion_props;
//
//		Float32 const friction = 0.5f * (a.friction + b.friction);
//
//		for (Uint32 i = 0; i < manifold.num_contacts; ++i) {
//
//			// points from a -> b
//			Vec3 const n = manifold.normal;
//			
//			Vec3 const ra = a.WorldToLocal( manifold.contacts[i].position );
//			Vec3 const rb = b.WorldToLocal( manifold.contacts[i].position );
//
//			Vec3 const prev_pa = mp_a ? mp_a->prev_position + glm::rotate(mp_a->prev_orientation, ra) : a.position;
//			Vec3 const prev_pb = mp_b ? mp_b->prev_position + glm::rotate(mp_b->prev_orientation, rb) : b.position;
//
//			Vec3 const ra_x_n = glm::cross(ra, n);
//			Vec3 const rb_x_n = glm::cross(rb, n);
//
//			Float32 const wa = mp_a ? mp_a->inv_mass + glm::dot(ra_x_n, mp_a->inv_inertia * ra_x_n) : 0.0f;
//			Float32 const wb = mp_b ? mp_b->inv_mass + glm::dot(rb_x_n, mp_b->inv_inertia * rb_x_n) : 0.0f;
//			
//			// Skip position and orientation correction if generalized masses are both zero
//			if (glm::epsilonEqual(wa, 0.0f, 0.0001f) && glm::epsilonEqual(wb, 0.0f, 0.0001f)) { continue; }
//
//			// ---------------------------------
//			// Correction along collision normal
//			// ---------------------------------
//			Float32 const c = manifold.contacts[i].penetration; // >0 => penetration
//			if (c <= 0.0f) { continue; } // sanity check
//
//			//Float32 const d_lambda_n = -c / (wa + wb);
//			//manifold.contacts[i].lambda_n += d_lambda_n;
//
//			//Vec3 const p = d_lambda_n * n;
//			Vec3 const p = -c * n;
//
//			// Apply position and orientation correction
//			if (mp_a) {
//				a.position += p * mp_a->inv_mass;
//				Vec3 const rot = mp_a->inv_inertia * glm::cross(ra, p);
//				Quat const dq = { 0, rot.x, rot.y, rot.z };
//				a.orientation += 0.5f * dq * a.orientation;
//			}
//			if (mp_b) {
//				b.position -= p * mp_b->inv_mass;
//				Vec3 const rot = mp_b->inv_inertia * glm::cross(rb, p);
//				Quat const dq = { 0, rot.x, rot.y, rot.z };
//				b.orientation -= 0.5f * dq * b.orientation;
//			}
//
//			// ---------------------
//			// Apply static friction
//			// ---------------------
//			//Vec3 const dv = (a.position - prev_pa) - (b.position - prev_pb); // relative motion at contact points, from a -> b -- check this!!
//			//Vec3 const dv_t = dv - glm::dot(dv, n) * n;
//			//Float32 const c_t = glm::length(dv_t);
//			//if (glm::epsilonEqual(c_t, 0.0f, 0.0001f)) { continue; }
//
//			//// Tangent direction
//			//Vec3 const t = dv_t / c_t;
//
//			//Float32 const d_lambda_t = -c_t / (wa + wb);
//			//manifold.contacts[i].lambda_t += glm::length(d_lambda_t);
//
//			//if (manifold.contacts[i].lambda_t < friction * manifold.contacts[i].lambda_n) {
//			//	Vec3 const p_t = d_lambda_t * t;
//
//			//	if (mp_a) {
//			//		a.position += p_t * mp_a->inv_mass;
//			//		Vec3 const rot = mp_a->inv_inertia * glm::cross(ra, p_t);
//			//		Quat const dq = { 0, rot.x, rot.y, rot.z };
//			//		a.orientation += 0.5f * dq * a.orientation;
//			//	}
//			//	if (mp_b) {
//			//		b.position -= p_t * mp_b->inv_mass;
//			//		Vec3 const rot = mp_b->inv_inertia * glm::cross(rb, p_t);
//			//		Quat const dq = { 0, rot.x, rot.y, rot.z };
//			//		b.orientation -= 0.5f * dq * b.orientation;
//			//	}
//			//}
//
//		}
//
//	}
//	
//	// Solve other constraints
//	SolveGroundConstraint();
//}
//
//void PhysicsManager::XPBDSolveVelocities(Float32 h) {
//	
//	for (auto&& [key, manifold] : narrow_phase_results) {
//		if (manifold.num_contacts == 0) { continue; }
//		
//		RigidBody& a = *key.a;
//		RigidBody& b = *key.b;
//
//		Float32 const friction = glm::sqrt(a.friction * b.friction);
//
//		MotionProperties* mp_a = a.motion_props;
//		MotionProperties* mp_b = b.motion_props;
//		for (Uint32 i = 0; i < manifold.num_contacts; ++i) {
//
//			// points from a -> b
//			Vec3 const n = manifold.normal;
//
//			Vec3 const ra = a.WorldToLocal(manifold.contacts[i].position);
//			Vec3 const rb = b.WorldToLocal(manifold.contacts[i].position);
//		
//			Vec3 const va = (mp_a) ? mp_a->linear_velocity : Vec3(0);
//			Vec3 const vb = (mp_b) ? mp_b->linear_velocity : Vec3(0);
//			Vec3 const wa = (mp_a) ? mp_a->angular_velocity : Vec3(0);
//			Vec3 const wb = (mp_b) ? mp_b->angular_velocity : Vec3(0);
//			Vec3 const v = (va + glm::cross(wa, ra)) - (vb + glm::cross(wb, rb));
//
//			// velocity in normal and tangent directions
//			Float32 const vn = glm::dot(n, v);
//			Vec3 const vt = v - n * vn;
//			Float32 const mag_vt = glm::length(vt);
//			if (glm::epsilonEqual(mag_vt, 0.0f, 0.001f)) { continue; }
//			
//			// normal force
//			Float32 const f_n = manifold.contacts[i].lambda_n / (h * h);
//
//			// relative velocity
//			Vec3 const dv = -vt * 1.0f/mag_vt * std::min(h * friction * std::abs(f_n), mag_vt);
//
//			Vec3 const p = dv * 1.0f / (wa + wb);
//			
//			if (mp_a) {
//				mp_a->linear_velocity += p * mp_a->inv_mass;
//				mp_a->angular_velocity += mp_a->inv_inertia * glm::cross(ra, p);
//
//				mp_a->linear_velocity *= (1.0f - mp_a->linear_damping);
//				mp_a->angular_velocity *= (1.0f - mp_a->angular_damping);
//			}
//			if (mp_b) {
//				mp_b->linear_velocity -= p * mp_b->inv_mass;
//				mp_b->angular_velocity -= mp_b->inv_inertia * glm::cross(rb, p);
//
//				mp_b->linear_velocity *= (1.0f - mp_b->linear_damping);
//				mp_b->angular_velocity *= (1.0f - mp_b->angular_damping);
//			}
//		}
//	}
//
//}
#pragma endregion