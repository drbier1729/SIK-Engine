#pragma once
#include "Engine\Component.h"

class Collectable;

class Inventory : public Component {
public:
	VALID_COMPONENT(Inventory);
	~Inventory();
	/*
	* Sets the max size of the inventory specified in the json
	* Returns: void
	*/
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value & json_value, rapidjson::MemoryPoolAllocator<>&alloc);

	/*
	* Adds a collectable to the inventory
	* Will not add any more collectables if the inventory is full
	* Returns: Bool - True if collectable added successfully
	*/
	Bool AddCollectable(Collectable*);

	/*
	* Gets the count of the collectables that are part of the inventory
	* Returns: Uint8 - Size of collectables
	*/
	Uint8 GetCollectableCount() const;
private:
	Vector<Collectable*> collectables;
	Uint8 max_size;
};

