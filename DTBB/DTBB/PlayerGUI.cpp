#include "stdafx.h"

#include "PlayerGUI.h"

#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"
#include "Engine/GameObject.h"

#include "Health.h"

PlayerGUI::PlayerGUI():
	player_health{ nullptr }
{
}

PlayerGUI::~PlayerGUI() noexcept
{
}

void PlayerGUI::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<PlayerGUI>(json_value, this);
}

void PlayerGUI::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<PlayerGUI>(json_value, this, alloc);
}

void PlayerGUI::Link() {
	player_health = p_gui_object_manager->GetTextObject(0);
}

void PlayerGUI::OnCollide(GameObject* other)
{
}

void PlayerGUI::Update(Float32 dt) {
	Health* p_health = GetOwner()->HasComponent<Health>();

	player_health->SetText(std::to_string(p_health->GetCurrHP()).c_str());
}

BEGIN_ATTRIBUTES_FOR(PlayerGUI)
END_ATTRIBUTES