#pragma once
#include "Engine\Component.h"
class Collectable : public Component {
public:
	enum class CTypes
	{
		Resource1,
		Resource2
	};

	VALID_COMPONENT(Collectable);
	~Collectable() = default;
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void OnCollide(GameObject* other) override;
	inline bool isCollected() const {
		return is_collected;
	}
	void Update(Float32 dt) override;
private:
	static std::unordered_map<StringID, CTypes> string_types_map;

	Bool is_collected = false;
	CTypes c_type;
};

