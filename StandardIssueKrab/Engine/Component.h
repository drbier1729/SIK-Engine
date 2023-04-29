#pragma once

// Each game will define this macro for its own set of components
#ifndef COMPONENT_LIST_FILE
#define COMPONENT_LIST_FILE "DefaultComponents.x"
#endif

#ifndef BUILDING_COMPONENT_LIST_FILE
#define BUILDING_COMPONENT_LIST_FILE "PrototypeBuilding\BuildingComponents.x"
#endif // !BUILDING_COMPONENT_LIST_FILE

#ifndef COMBAT_COMPONENT_LIST_FILE
#define COMBAT_COMPONENT_LIST_FILE "PrototypeQuesting\CombatComponents.x"
#endif // !COMBAT_COMPONENT_LIST_FILE

#ifndef DRIVING_COMPONENT_LIST_FILE
#define DRIVING_COMPONENT_LIST_FILE "PrototypeDriving\DrivingComponents.x"
#endif // !DRIVING_COMPONENT_LIST_FILE

#ifndef GAME_COMPONENT_LIST_FILE
#define GAME_COMPONENT_LIST_FILE "..\DTBB\DTBB\GameComponents.x"
#endif // !GAME_COMPONENT_LIST_FILE


//Forward declaration
class GameObject;

/*
* Base component class that serves as the generic component
* Each individual component will need to implement the virtual methods
* Any behavior will have to be contained within the individual components.
*/
class Component {

public:
	enum class Type {
		#define REGISTER_COMPONENT(component) component,
		#include COMPONENT_LIST_FILE
#ifndef _PROTOTYPE
		#include GAME_COMPONENT_LIST_FILE
#else
		#include BUILDING_COMPONENT_LIST_FILE
		#include COMBAT_COMPONENT_LIST_FILE
		#include DRIVING_COMPONENT_LIST_FILE
#endif // _PROTOTYPE
		#undef REGISTER_COMPONENT
		INVALID
	};
	static constexpr SizeT NUM_COMPONENTS{ static_cast<SizeT>(Type::INVALID) };

private:
	GameObject* p_owner_object;

public:
	// Ctors defaulted
	virtual ~Component();

	//Base Update method. Does nothing.
	virtual void Update(Float32 dt);

	//Base FixedUpdate method. Does nothing.
	virtual void FixedUpdate(Float32 fixed_dt);

	// Abstract Deserialize method.
	virtual void Deserialize(rapidjson::Value const& json_value) = 0;

	// Abstract Modify method. Calls Deserialize.
	virtual void Modify(rapidjson::Value const& json_value);

	// Abstract Serialize method.
	virtual void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) = 0;

	// Base Collision handler method. Does nothing.
	virtual void OnCollide(GameObject* other) {}

	//Base GetType method. Returns Component::Type::INVALID.
	virtual constexpr Type GetType() const;

	//Returns the name of the component as C string. Returns "INVALID".
	virtual constexpr const char* GetName() const;

	//Returns the name of the component as StringID. Returns "INVALID"_sid.
	virtual constexpr StringID GetNameSID() const;

	//Sets a GameObject as the owner of the component instance
	void SetOwner(GameObject* p_owner_object);

	//Sets a GameObject as the owner of the component instance
	GameObject* GetOwner();

	/*
	* Base link function. Does nothing
	* Links the component with other components that are 
	* related to it for the same game object
	* Returns: void
	*/
	virtual void Link();

	/*
	* Base enable function. Does nothing.
	* Returns: void
	*/
	virtual void Enable();

	/*
	* Base disable function. Does nothing.
	* Returns: void
	*/
	virtual void Disable();

	/*
	* Resets the component to some starting state
	* Retruns: void
	*/
	virtual void Reset();
};

// Runtime helpers to access Component "Type" from a string or StringID
Component::Type ComponentTypeFromName(StringID name_sid);
Component::Type ComponentTypeFromName(const char* name);


// Concept which all components must satisfy
template<class T>
concept ValidComponent = std::derived_from<T, Component> && requires {
	{ T::type } -> std::convertible_to<Component::Type>;
	std::same_as<std::remove_cvref_t<decltype(T::type)>, Component::Type>;

	{ T::name_sid } -> std::convertible_to<StringID>;
	std::same_as<std::remove_cvref_t<decltype(T::name_sid)>, StringID>;

	{ T::name } -> std::convertible_to<const char*>;
	std::same_as<std::remove_cvref_t<decltype(T::name)>, const char*>;
};

// Assigns a type to the component. Must use this in the header/declaration for each game-specific component.
#ifndef VALID_COMPONENT
#define VALID_COMPONENT(component) \
static constexpr Component::Type type{ Component::Type::##component }; \
constexpr Component::Type GetType() const override { return type; } \
static constexpr StringID name_sid{ #component##_sid }; \
constexpr StringID GetNameSID() const override { return name_sid; } \
static constexpr const char* name{ #component }; \
constexpr const char* GetName() const override { return name; }
#endif