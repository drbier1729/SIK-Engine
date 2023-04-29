#include "stdafx.h"
#include "Behaviour.h"

#include "ScriptingManager.h"

Behaviour::Behaviour()
	: is_active{ true }, owner{ nullptr }, scripts{}
{}

Behaviour::Behaviour(PolymorphicAllocator alloc_)
	: scripts{ alloc_ }, is_active{ true }, owner{ nullptr }
{}


void Behaviour::AddScript(const char* script_name) {
	scripts.emplace_back(script_name);
}

void Behaviour::RemoveScript(const char* script_name) {
	std::erase_if(scripts,
		[script_name](Script const& s) {
			return std::strcmp(s.script_name, script_name) == 0;
		}
	);
}

void Behaviour::LoadScripts() {
	for (auto& script : scripts) {
		script.script_state.open_libraries(sol::lib::base);
		// register functions
		p_scripting_manager->RegisterGlobals(script.script_state);

		if (owner)
			p_scripting_manager->RegisterGameObjectFunctions(script.script_state, owner);

		if (owner_gui)
			p_scripting_manager->RegisterGUIFunctions(script.script_state, owner_gui);

		if (owner_cam)
			p_scripting_manager->RegisterCameraFunctions(script.script_state, owner_cam);

		// load script
		std::string script_file = "..\\StandardIssueKrab\\Engine\\Assets\\Scripts\\" + std::string{ script.script_name };
		script.script_result = script.script_state.load_file(script_file);
	}
}

void Behaviour::LoadSingleScript(const char* script_name) {
	for (auto& script : scripts) {
		// find the script
		if (std::strcmp(script.script_name, script_name) != 0) continue;

		// register functions
		p_scripting_manager->RegisterGlobals(script.script_state);
		p_scripting_manager->RegisterGameObjectFunctions(script.script_state, owner);

		// load script
		std::string script_file = "..\\StandardIssueKrab\\Engine\\Assets\\Scripts\\" + std::string{ script.script_name };
		script.script_result = script.script_state.load_file(script_file);
		break;
	}
}

void Behaviour::Update(Float32 dt) {
	if (not is_active)
		return;

	for (auto& script : scripts) {
		// set dt
		script.script_state.set("dt", dt);

		// run once per frame
		sol::protected_function_result result = script.script_result();

		auto thing =  script.script_state["Vec2"].valid();
		if (!result.valid()) {
			sol::error err = result;
			SIK_ERROR("Script error : \"{}\"", err.what());
			SIK_ASSERT(false, "Error in script");
		}

		//Reset the has_collided flag
		script.script_state.set("has_collided", false);
	}
}


/*
* Function to set the has_collided flag to true in case of Physics collision
* Returns: void
*/
void Behaviour::SetCollided() {
	if (not is_active)
		return;

	for (auto& script : scripts) {
		//Set the has_collided flag to true
		script.script_state["has_collided"] = true;
	}
}

