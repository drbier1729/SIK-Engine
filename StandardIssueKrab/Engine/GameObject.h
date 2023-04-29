#pragma once
#include "Component.h"
#include "Transform.h"
#include "RigidBody.h"
#include "MeshRenderer.h"
#include "Behaviour.h"

/*
* Set this at compile-time to tune how components are accessed and stored. 
* If defined as 1: components will be stored in a dense vector and accessed 
* linearly.
* If defined as 0: components will be stored in a sparse vector and accessed by
* index.
* Here we make the default Linear Access
*/
#ifndef COMPONENT_LINEAR_ACCESS
#define COMPONENT_LINEAR_ACCESS 1
#endif


/*
* A GameObject class.
* Essentially just a list of components.
* The GameObject itself does NOT define any behavior.
* All behavior needs to be contained in components.
*/
class GameObject {
public:
	friend class PhysicsManager;
	friend class ScriptingManager;
private:
	bool is_active;
	Transform transform;

	RigidBody* rigidbody;
	MeshRenderer* mesh_renderer;
	Behaviour* behaviour;
	// Ptrs to other engine-side "components"
	// ...

	Vector<UniquePtr<Component>> game_components; // game-specific components
	String name;
public:
	//Creates a named game object
	explicit GameObject(const char* _name = "<NO NAME>", PolymorphicAllocator _obj_alloc = {});

	~GameObject();

	void OnCollide(GameObject* other);

	//Checks if the game object is active
	inline bool IsActive() const;

	//Disable game object
	void Disable();

	//Disable all the functionality but still render the object
	void DisableExceptRender();

	//Enable game object
	void Enable();

	//Sets the name of the game object
	inline void SetName(const char* _name);

	//Returns the name of the Game object
	inline const String& GetName() const;

	//Gets a reference to the internal vector of component ptrs
	inline Vector<UniquePtr<Component>>& GetComponentArray();
	inline const Vector<UniquePtr<Component>>& GetComponentArray() const;

	//Checks if the game object has a particular component. Returns it if present.
	//Prefer to use this method over the other HasComponent overloads since this
	//one will give a compile-time error if you attempt to access an invalid
	//component type, and will handle the pointer cast for you.
	template<typename C> C* HasComponent(); // intentionally unimplemented
	template<ValidComponent C> C* HasComponent();
	template<> inline RigidBody*  HasComponent<RigidBody>();
	template<> inline Transform*  HasComponent<Transform>();
	template<> inline MeshRenderer* HasComponent<MeshRenderer>();
	template<> inline Behaviour* HasComponent<Behaviour>();

	//Adds a component to the list of components of the game object.
	void AddComponent(Component* component);
	void AddComponent(RigidBody* rb);
	void AddComponent(MeshRenderer* mr);
	void AddComponent(Behaviour* bh);

	//Calls the Update() function for each of its components
	void Update(Float32 dt);

	//Calls the FixedUpdate() function for each of its components
	void FixedUpdate(Float32 fixed_dt);


	///////////////////////////////////////
	// Helpers and special use case methods
	///////////////////////////////////////
	
	// Note that these will NOT work with engine-side components (e.g. RigidBody, Behaviour)
	Component* HasComponent(Component::Type comp_type);
	Component* HasComponent(const char* component_name);
	Component* HasComponent(StringID component_name_sid);

	/*
	* Removes the most recently added component
	* **Only use when removing components added from the prototypes**
	*/
	Component* RemoveLastComponent();

	/*
	* Links the related components.
	* Must be called after all the components have been added to the game object
	*/
	void Link();

	/*
	* Creates a copy of this game object along with all of its components
	*/
	//GameObject Clone() const;

	/*
	* Resets the game object to its starting state by calling Reset()
	* on all components
	* Returns: void
	*/
	void Reset();
};

// Inline definitions
inline bool GameObject::IsActive() const {
	return is_active;
}

inline void GameObject::SetName(const char* _name) {
	name = _name;
}

inline const String& GameObject::GetName() const {
	return name;
}

inline Vector<UniquePtr<Component>>& GameObject::GetComponentArray() {
	return game_components;
}
inline const Vector<UniquePtr<Component>>& GameObject::GetComponentArray() const {
	return game_components;
}

template<ValidComponent C>
C* GameObject::HasComponent() {
	return static_cast<C*>( HasComponent(C::type) );
}

template<> inline RigidBody* GameObject::HasComponent<RigidBody>() { return rigidbody; }
template<> inline Transform* GameObject::HasComponent<Transform>() { return &transform; }
template<> inline MeshRenderer* GameObject::HasComponent<MeshRenderer>() { return mesh_renderer; }
template<> inline Behaviour* GameObject::HasComponent<Behaviour>() { return behaviour; }
