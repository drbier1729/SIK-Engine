#include "stdafx.h"

#include "Component.h"


Component::~Component() {}

//Sets a GameObject as the owner of the component instance
void Component::SetOwner(GameObject* owner_object) {
	p_owner_object = owner_object;
}

//Returns a pointer to the owner GameObject
GameObject* Component::GetOwner() {
	return p_owner_object;
}

/*
* Links the component with other components that are
* related to it for the same game object
* Returns: void
*/
void Component::Link() {
}

void Component::Enable() {
}


void Component::Disable() {
}

void Component::Reset() {
}

//Base Update method. Does nothing.
void Component::Update(Float32) {

}

//Base Update method. Does nothing.
void Component::FixedUpdate(Float32) {

}

void Component::Modify(rapidjson::Value const& json_value) {
	Deserialize(json_value);
}

//Base method
constexpr const char* Component::GetName() const {
	return "INVALID";
}

//Base method
constexpr StringID Component::GetNameSID() const {
	return "INVALID"_sid;
}

//Base method
constexpr Component::Type Component::GetType() const { 
	return Type::INVALID; 
}

Component::Type ComponentTypeFromName(StringID name_sid) {
	switch (name_sid) {
		#define REGISTER_COMPONENT(component) break; case #component##_sid : { return Component::Type::##component; }
		#include COMPONENT_LIST_FILE
		#ifndef _PROTOTYPE
		#include GAME_COMPONENT_LIST_FILE
		#else
		#include BUILDING_COMPONENT_LIST_FILE
		#include COMBAT_COMPONENT_LIST_FILE
		#include DRIVING_COMPONENT_LIST_FILE
		#endif // _PROTOTYPE
		#undef REGISTER_COMPONENT
		break; default: {}
	}

	return Component::Type::INVALID;
}

Component::Type ComponentTypeFromName(const char* name) {
	return ComponentTypeFromName(ToStringID(name));
}