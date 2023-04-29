#include "stdafx.h"
#include "Inventory.h"

#include "Engine/MemoryManager.h"
#include "Engine/Serializer.h"
#include "Engine/GameObjectManager.h"

Inventory::Inventory() {
	collectables[Collectable::CTypes::Resource1] = 0;
	collectables[Collectable::CTypes::Resource2] = 0;
}

Inventory::~Inventory() {
	collectables.clear();
}

/*
* Sets the max size of the inventory specified in the json
* Returns: void
*/
void Inventory::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Inventory>(json_value, this);
}

void Inventory::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Inventory>(json_value, this, alloc);
}

/*
* Adds a collectable to the inventory
* Will not add any more collectables if the inventory is full
* Returns: Bool - True if collectable added successfully
*/
Bool Inventory::AddCollectable(Collectable* _collectable) {	
	auto& count = collectables[_collectable->GetCType()];

	using NumT = std::remove_cvref_t<decltype(count)>;

	if (count == std::numeric_limits<NumT>::max()) {
		return false;
	}

	count++;
	return true;
}


Bool Inventory::AddCollectable(Collectable::CTypes type, Uint16 n) {
	auto& count = collectables[type];
	using NumT = std::remove_cvref_t<decltype(count)>;


	auto const remaining_space = (NumT)(std::numeric_limits<NumT>::max() - count);
	if (n > remaining_space) {
		n = remaining_space;
	}
	
	count += n;
	return true;
}

/*
* Gets the count of the collectables that are part of the inventory
* Returns: Uint8 - Size of collectables
*/
Uint16 Inventory::GetCollectableCount(Collectable::CTypes collectable_type) const {
	return collectables.at(collectable_type);
}

void Inventory::RemoveCollectables(Collectable::CTypes collectable_type, Uint16 remove_count) {
	collectables[collectable_type] -= remove_count;
}

BEGIN_ATTRIBUTES_FOR(Inventory)
END_ATTRIBUTES
