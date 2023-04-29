#pragma once
#include "Engine\Component.h"

// forward decl
struct ParticleEmitter;

class Collectable : public Component {
public:
	enum class CTypes
	{
		Resource1,
		Resource2
	};

	VALID_COMPONENT(Collectable);
	ALLOW_PRIVATE_REFLECTION;

	Collectable();
	~Collectable() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Enable() override;
	void Disable() override;

	void OnCollide(GameObject* other) override;
	void Update(Float32 dt) override;

	void SetTrailColor(Vec3 const& color);

	inline Bool isCollected() const {
		return is_collected;
	}
	
	inline void SetStartHeight(Float32 height) { starting_height = height; }

	inline CTypes GetCType() { return c_type; }
private:
	static std::unordered_map<StringID, CTypes> string_types_map;

	ParticleEmitter* p_emitter;

	Bool is_collected;
	CTypes c_type;

	Float32 elapsed_time;

	// Serializable members
	Float32 rising_speed;
	Float32 chase_speed;
	Float32 apex_threshold;
	Float32 starting_height;
};

