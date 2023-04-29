#pragma once

#include <sol/sol.hpp>
struct Script;
class Behaviour {
public:
	// ctor and dtor
	Behaviour();
	explicit Behaviour(PolymorphicAllocator _alloc);
	~Behaviour() noexcept = default;

	// not needed
	Behaviour(const Behaviour&) = delete;
	Behaviour& operator=(const Behaviour&) = delete;
	Behaviour(Behaviour&&) = delete;
	Behaviour& operator=(Behaviour&&) = delete;

	// adding a script to the vector
	void AddScript(const char* script_name);

	// remove a script 
	void RemoveScript(const char* script_name);

	// load scripts to be run
	void LoadScripts();

	// load single script
	void LoadSingleScript(const char* script_name);

	// run script
	void Update(Float32 dt);

	/*
	* Function to set the has_collided flag to true in case of Physics collision
	* Returns: void
	*/
	void SetCollided();

	/*
	* Sets a state variable on all the scripts
	* Returns: void
	*/
	template<class T>
	void SetStateVariable(T variable, const char* var_name);
public:
	class GameObject* owner = nullptr;
	class GUIObject* owner_gui = nullptr;
	class RenderCam* owner_cam = nullptr;
	Bool is_active = false;

	struct Script {
		const char*			script_name;
		sol::state			script_state;
		sol::load_result	script_result;

		Script(const char* _script_name):
			script_name{ _script_name },
			script_state{},
			script_result{}
		{}
	};
	Vector<Script> scripts;
};

template<class T>
inline void Behaviour::SetStateVariable(T variable, const char* var_name) {
	for (auto& script : scripts) {
		script.script_state.set(var_name, variable);
	}
}