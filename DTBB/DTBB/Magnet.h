#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declaration
class Debris;

class Magnet : public Component {
public:
	VALID_COMPONENT(Magnet);
	ALLOW_PRIVATE_REFLECTION;

	Magnet();
	~Magnet() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Modify(rapidjson::Value const& json_value) override;

	void OnCollide(GameObject* other) override;
	void Link() override;

	void Enable() override;
	void Disable() override;

	void Reset() override;

	void Update(Float32 dt);
	void FixedUpdate(Float32 dt);

	void AddDebris(Debris* p_debris);
	Bool HasFreeSlots();

	// called from magnet script
	void ShootDebris();

	Int32 GetSoundID();
	void SetNumAttached(Uint16 _num_attached_debris);
	void PlayLoadedSound(Bool _play_loaded_sound);

private:

private:
	// tuple<local pos around magnet, ptr to debris>
	Vector<Tuple<Vec3, Debris*>> attached_debris;

	// currently attached debris objects
	Uint16 num_attached_debris;
	static constexpr Uint8 MAX_ATTACHED_DEBRIS = 13;

	// sound effects
	Int32 sound_id;
	Bool play_loaded_sound;

	Vec3 orig_pos;
	Quat orig_ori;

	// serializable members
	Float32 shoot_speed;
};