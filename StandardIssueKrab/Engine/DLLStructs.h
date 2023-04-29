#pragma once

//Forward declarations
class GameManager;
class GraphicsManager;
class InputManager;
class GameObjectManager;
class ResourceManager;
class Factory;
class PhysicsManager;
class GUIObjectManager;
class ParticleSystem;
class ScriptingManager;
class AudioManager;
class MemoryManager;
class WorldEditor;
class FrameTimer;
class GameStateManager;
namespace SIK { class ImGuiWindow; }

typedef struct {
    GameManager* p_engine_game_manager;
    GameObjectManager* p_engine_game_obj_manager;
    GraphicsManager* p_engine_graphics_manager;
    InputManager* p_engine_input_manager;
    ResourceManager* p_engine_resource_manager;
    Factory* p_engine_factory;
    PhysicsManager* p_engine_physics_manager;
    GUIObjectManager* p_engine_gui_obj_manager;
    ParticleSystem* p_engine_particle_system;
    ScriptingManager* p_engine_scripting_manager;
    AudioManager* p_engine_audio_manager;
    WorldEditor* p_engine_world_editor;
    MemoryManager* p_engine_memory_manager;
    SIK::ImGuiWindow* p_engine_imgui_window;
    FrameTimer* p_engine_frame_timer;
    GameStateManager* p_gamestate_manager;

#ifdef STR_DEBUG
    StringDictionary* p_dbg_string_dictionary;
#endif
} EngineExport;
