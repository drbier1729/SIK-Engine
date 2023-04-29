#pragma once

#include "Component.h"
#include "MemoryManager.h"

class GameObject;

using ComponentBuilder = Component*(*)();

class Factory {
	UnorderedMap<StringID, ComponentBuilder> builder_map;

private:
	void SaveObjectInScene(rapidjson::Document& doc, GameObject const& go);

public:
	Factory();
	~Factory() noexcept = default;

	Factory(const Factory&) = delete;
	Factory& operator=(const Factory&) = delete;
	Factory(Factory&&) = delete;
	Factory& operator=(Factory&&) = delete;

	template<ValidComponent C>
	void RegisterComponent();
	
	GameObject* BuildGameObject(const char* filename);
	GameObject* BuildGameObject(rapidjson::Document& val);
	void SaveObjectArchetype(const char* archetype_filename, GameObject const& go);

	Vector<GameObject*> BuildScene(const char* filename);

	Component* GetGameSideComp(StringID comp_id);
};

//Declared as an extern variable so it can be accessed throughout the project
extern Factory* p_factory;


// Better way to do this?
template<ValidComponent C>
Component* Builder() {
	return MemoryManager::GetDefaultAllocator().new_object<C>();
}

// Inline definitions
template<ValidComponent C>
void Factory::RegisterComponent() {
	builder_map.emplace(C::name_sid, &Builder<C>);
}
