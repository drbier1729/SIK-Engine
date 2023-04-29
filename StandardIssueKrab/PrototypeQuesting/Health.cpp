#include "stdafx.h"

#include "Health.h"

#include "Engine/ResourceManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"

void Health::Update(Float32 dt) {
	// update GUI with health
	p_gui_object_manager->GetTextObject(3)->SetText(std::to_string(health).c_str());
}

void Health::TakeDamage(Int32 damage) {
	health -= damage;
}

BEGIN_ATTRIBUTES_FOR(Health)
	DEFINE_MEMBER(Int32, health)
END_ATTRIBUTES