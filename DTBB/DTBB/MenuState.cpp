#include "stdafx.h"

#include "MenuState.h"
#include "StartState.h"

#include "Engine/GUIObjectManager.h"
#include "Engine/GUIObject.h"
#include "Engine/ScriptingManager.h"
#include "Engine/GameStateManager.h"

#include "ScriptInterface.h"

MenuState::MenuState(const char* menu_json) {
	menu_objects = p_gui_object_manager->CreateGUIFromFile(menu_json);

	for (auto& gui_object : menu_objects) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaStartState(script.script_state,
					p_start_state);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {

					RegisterLuaStartState(script.script_state,
						p_start_state);
				}
			}
		}
	}
}

MenuState::~MenuState() {
	
}

void MenuState::Enter() {
	for (auto& obj : menu_objects)
		obj->Enable();
}

void MenuState::Exit() {
	for (auto& obj : menu_objects)
		obj->Disable();

	p_gui_object_manager->RemoveAllHighlightObjects();
}

void MenuState::Update(Float32 dt) {
	p_gui_object_manager->UpdateUIControls(dt);

	for (auto& obj : menu_objects)
		obj->Update(dt);
}