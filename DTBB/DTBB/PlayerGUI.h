#pragma once

#include "Engine/Component.h"
#include "Engine/Serializer.h"

// forward declarations
class GUIText;

class PlayerGUI : public Component {
public:
	VALID_COMPONENT(PlayerGUI);
	ALLOW_PRIVATE_REFLECTION;

	PlayerGUI();
	~PlayerGUI() noexcept;

	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);

	void Link() override;
	void OnCollide(GameObject* other) override;

	void Update(Float32 dt);

private:
	GUIText* player_health;
};