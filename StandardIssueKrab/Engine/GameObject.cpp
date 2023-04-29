#include "stdafx.h"
#include "GameObject.h"

#include "PhysicsManager.h"
#include "FrameRateManager.h"
#include "ScriptingManager.h"
#include "MemoryManager.h"

//TODO: Figure out an efficient way of generating uuids in runtime

/*
* Creates a named game object using the given allocator.
* Essentially is only called by the gameobject manager which
* passes it's own allocator.
*/
GameObject::GameObject(const char* _name, PolymorphicAllocator obj_alloc) :
	name(_name, obj_alloc), is_active(true), transform(), rigidbody(nullptr),
	game_components(obj_alloc) {
}

GameObject::~GameObject()
{
	if (mesh_renderer != nullptr)
	{
		mesh_renderer->is_valid = false;
		mesh_renderer->owner = nullptr;
		mesh_renderer = nullptr;
	}
	if (rigidbody != nullptr)
	{
		rigidbody->owner = nullptr;
		p_physics_manager->RemoveRigidBody(rigidbody); // mark as invalid for deletion next frame
		rigidbody = nullptr;
	}
	if (behaviour != nullptr) {
		p_scripting_manager->DeleteBehaviour(behaviour);
		behaviour = nullptr;
	}

	//Delete the Components created by the builder
	//For some reason the UniquePointers don't call the pmr delete_object
	//so that reports a leak whenever we have components.
	for (auto& component : game_components) {
		MemoryManager::GetDefaultAllocator().delete_object(
			component.release()
		);
	}
}

void GameObject::Enable() {
	is_active = true;
	MeshRenderer* mr = HasComponent<MeshRenderer>();
	if (mr) {
		mr->enabled = true;
	}
	
	RigidBody* rb = HasComponent<RigidBody>();
	if (rb) {
		rb->Enable(true);
	}

	Behaviour* bh = HasComponent<Behaviour>();
	if (bh) {
		bh->is_active = true;
	}

	for (auto&& comp : game_components) {
		comp->Enable();
	}
}

void GameObject::Disable() {
	MeshRenderer* mr = HasComponent<MeshRenderer>();
	if (mr) {
		mr->enabled = false;
	}

	RigidBody* rb = HasComponent<RigidBody>();
	if (rb) {
		rb->Enable(false);
	}

	Behaviour* bh = HasComponent<Behaviour>();
	if (bh) {
		bh->is_active = false;
	}

	for (auto&& comp : game_components) {
		comp->Disable();
	}
}

void GameObject::DisableExceptRender() {
	RigidBody* rb = HasComponent<RigidBody>();
	if (rb) {
		rb->Enable(false);
	}

	Behaviour* bh = HasComponent<Behaviour>();
	if (bh) {
		bh->is_active = false;
	}

	for (auto&& comp : game_components) {
		comp->Disable();
	}
}

void GameObject::OnCollide(GameObject* other) {
	/*Check if the gameobject has a behavior
	* If so, set the collided state
	*/
	Behaviour* bh = HasComponent<Behaviour>();
	if (bh) {
		bh->SetCollided();
	}

	for (auto&& comp : game_components) {
		comp->OnCollide(other);
	}
}


Component* GameObject::HasComponent(Component::Type comp_type) {
#if COMPONENT_LINEAR_ACCESS
	auto it = std::find_if(
		game_components.begin(),
		game_components.end(),
		[&comp_type](UniquePtr<Component> const& comp) {
			return comp->GetType() == comp_type;
		});

	if (it != game_components.end()) {
		return it->get();
	}

	return nullptr;

#else
	return game_components[static_cast<SizeT>(comp_type)].get();

#endif
}

/*
* Converts the name into StringID for checking against the map
* Returns: Pointer to the component or nullptr of Component doesn't exist
*/
Component* GameObject::HasComponent(const char* component_name) {
	return HasComponent(ComponentTypeFromName(component_name));
}

/*
* Returns: Pointer to the component or nullptr of Component doesn't exist
*/
Component* GameObject::HasComponent(StringID component_name_sid) {
	return HasComponent(ComponentTypeFromName(component_name_sid));
}

/*
* Removes the most recently added component
* **Only use when removing components added from the prototypes**
*/
Component* GameObject::RemoveLastComponent() {
#if COMPONENT_LINEAR_ACCESS
	auto it = game_components.rbegin();
	return it->release();
#else
	game_components.pop_back();
#endif
}

/*
* Links the related components.
* Must be called after all the components have been added to the game object
*/
void GameObject::Link() {
	for (auto&& comp : game_components) {
		comp->Link();
	}
}

void GameObject::Reset() {
	for (auto&& comp : game_components) {
		comp->Reset();
	}
}

/*
* Takes a pointer to a component which is created by a component creator.
* Adds that to the component_map with the StringID for the component name as the key.
* Sets the components owner as the game object
* Return: void
*/
void GameObject::AddComponent(Component* component) {
	component->SetOwner(this);

#if COMPONENT_LINEAR_ACCESS
	auto comp_type = component->GetType();
	for (auto&& c : game_components) {
		if (c->GetType() == comp_type) {
			c.reset(component);
			return;
		}
	}
	game_components.emplace_back(component);

#else
	SizeT index = static_cast<SizeT>(component->GetType());
	if (game_components.size() < index + 1) {
		game_components.resize(index + 1);
	}
	game_components[index].reset(component);
#endif
}

void GameObject::AddComponent(RigidBody* rb) {
	SIK_ASSERT(rb != nullptr, "Use PhysicsManager::RemoveRigidBody to remove a RigidBody.");
	SIK_ASSERT(rigidbody == nullptr, "This GameObject already owns a RigidBody. Remove its RigidBody using PhysicsManager::RemoveRigidBody before adding a new one.");
	SIK_ASSERT(rb->owner == nullptr, "This RigidBody is already assigned to another GameObject");
	rb->owner = this;
	rigidbody = rb;

	if (rigidbody->IsStatic()) {
		rigidbody->position = transform.position;
		rigidbody->orientation = transform.orientation;
		rigidbody->UpdateAABB();
	}
}

void GameObject::AddComponent(MeshRenderer* mr) {
	SIK_ASSERT(mr != nullptr, "Use GraphicsManager::DestroyMeshRenderer to remove a MeshRenderer");
	SIK_ASSERT(mesh_renderer == nullptr, "This GameObject already owns a MeshRenderer. Use GraphicsManager::DestroyMeshRenderer before adding a new one");
	SIK_ASSERT(mr->owner == nullptr, "This MeshRenderer is already assigned to another GameObject");
	mr->owner = this;
	mesh_renderer = mr;
}

void GameObject::AddComponent(Behaviour* bh) {
	SIK_ASSERT(bh->owner == nullptr, "This Behaviour is already assigned to another GameObject");
	bh->owner = this;
	behaviour = bh;
}

//Calls Update on all components
void GameObject::Update(Float32 dt) {
	if (not is_active)
		return;

	if (behaviour != nullptr) { behaviour->Update(dt); }

#if COMPONENT_LINEAR_ACCESS
		std::for_each(
			game_components.begin(), 
			game_components.end(), 
			[dt](UniquePtr<Component> const& ptr) {
				ptr->Update(dt);
			});
#else
		std::for_each(
			game_components.begin(),
			game_components.end(),
			[dt](UniquePtr<Component> const& ptr) {
				if (ptr != nullptr) { ptr->Update(dt); }
			});
#endif
}

void GameObject::FixedUpdate(Float32 fixed_dt) {
	if (not is_active)
		return;

	//if (behaviour != nullptr) { behaviour->FixedUpdate(fixed_dt); }

#if COMPONENT_LINEAR_ACCESS
		std::for_each(
			game_components.begin(), 
			game_components.end(), 
			[fixed_dt](UniquePtr<Component> const& ptr) {
				ptr->FixedUpdate(fixed_dt);
			});
#else
		std::for_each(
			game_components.begin(),
			game_components.end(),
			[dt](UniquePtr<Component> const& ptr) {
				if (ptr != nullptr) { ptr->FixedUpdate(fixed_dt); }
			});
#endif
}


