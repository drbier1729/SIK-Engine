#pragma once
#include "Engine\Component.h"

#include "Collectable.h"

class Inventory : public Component {
public:
	VALID_COMPONENT(Inventory);
	ALLOW_PRIVATE_REFLECTION;
	Inventory();
	~Inventory();
	/*
	* Sets the max size of the inventory specified in the json
	* Returns: void
	*/
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	/*
	* Adds a collectable to the inventory
	* Will not add any more collectables if the inventory is full
	* Returns: Bool - True if collectable added successfully
	*/
	Bool AddCollectable(Collectable*);
	
	/*
	* Adds a collectable to the inventory
	* Will not add any more collectables if the inventory is full
	* Returns: Bool - True if collectable added successfully
	*/
	Bool AddCollectable(Collectable::CTypes type, Uint16 count = 1);

	/*
	* Gets the count of the collectables that are part of the inventory
	* Returns: Uint8 - Size of collectables
	*/
	Uint16 GetCollectableCount(Collectable::CTypes collectable_type) const;

	/*
	* Removes a number of collectables
	* Returns: void
	*/
	void RemoveCollectables(Collectable::CTypes collectable_type, Uint16 remove_count);
private:
	UnorderedMap<Collectable::CTypes, Uint16> collectables;
};

