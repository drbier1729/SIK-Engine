#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

#include "FixedObjectPool.h"
#include "Behaviour.h"

// forward decls
class GameObject;
class InputAction;
class Behaviour;
class GUIObject;
class RenderCam;

class ScriptingManager {
public:
	ScriptingManager() = default;
	~ScriptingManager() = default;

	ScriptingManager(const ScriptingManager&) = delete;
	ScriptingManager& operator=(const ScriptingManager&) = delete;
	ScriptingManager(ScriptingManager&&) = delete;
	ScriptingManager& operator=(ScriptingManager&&) = delete;

	Behaviour* CreateBehaviour();
	Behaviour* CreateBehaviour(GUIObject* gui_object);
	Behaviour* CreateBehaviour(RenderCam* cam_object);
	void DeleteBehaviour(Behaviour* p_behaviour);
	/*
	* Calls clear on the underlying mem resource
	* Returns: void
	*/
	void DeleteAllBehaviours();

	void RegisterGlobals(sol::state& state) const;
	void RegisterGameObjectFunctions(sol::state& state, GameObject* p_game_obj) const;
	void RegisterGUIFunctions(sol::state& state, GUIObject* p_gui_object) const;
	void RegisterCameraFunctions(sol::state& state, RenderCam* p_cam_obj) const;
	void RegisterActionMap(sol::state& state, InputAction* input_action) const;

	template<class T>
	void SetStateVariable(sol::state& state, T variable, const char* var_name);

	/*
	* Get the list of all behaviours
	* Returns: Vector<Behavior*>&
	*/
	Vector<Behaviour*>& GetBehaviorListRef();
private:
	FixedObjectPool<Behaviour, 2048> script_pool;
	Vector<Behaviour*> behaviours_list;
private:
	void RegisterActionsEnum(sol::state& state) const;
};

//Declared as an extern variable so it can be accessed throughout the project
extern ScriptingManager* p_scripting_manager;

template<class T>
inline void ScriptingManager::SetStateVariable(
	sol::state& state, T variable, const char* var_name) {
	state.set(var_name, variable);
}
