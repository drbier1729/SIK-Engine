#include "stdafx.h"
#include "Inventory.h"
#include "Engine/MemoryManager.h"

Inventory::~Inventory() {
	collectables.clear();
}

/*
* Sets the max size of the inventory specified in the json
* Returns: void
*/
void Inventory::Deserialize(rapidjson::Value const& json_value) {
	max_size = json_value.FindMember("MaxSize")->value.GetInt();
}

void Inventory::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc)
{
}

/*
* Adds a collectable to the inventory
* Will not add any more collectables if the inventory is full
* Returns: Bool - True if collectable added successfully
*/
Bool Inventory::AddCollectable(Collectable* _collectable) {
	if (collectables.size() == max_size)
		return false;

	collectables.push_back(_collectable);
	return true;
}

/*
* Gets the count of the collectables that are part of the inventory
* Returns: Uint8 - Size of collectables
*/
Uint8 Inventory::GetCollectableCount() const {
	// size guaranteed to be <= max_size
	return static_cast<Uint8>(collectables.size());
}
