#include "stdafx.h"
#include "EngineMain.h"

#include "MemoryManager.h"
#include "MemoryResources.h"
#include "GameManager.h"
#include "FrameTimer.h"
#include "ImGuiWindow.h"
#include "InputManager.h"
#include "LogManager.h"
#include "GraphicsManager.h"
#include "PhysicsManager.h"
#include "DLLStructs.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include "WorldEditor.h"
#include "Factory.h"
#include "FrameRateManager.h"
#include "ParticleSystem.h"
#include "ScriptingManager.h"
#include "GUIObjectManager.h"
#include "GameStateManager.h"
#include "GameState.h"
#include "MenuState.h"
#include "PlayState.h"
#include "InputAction.h"
#include "RenderCam.h"


#ifdef MEM_DEBUG
static UniquePtr<DebugMemoryResource> noisy_allocator{ nullptr };
#endif


// Helpers to read/write from SIK_CONFIG.json
static Bool ConfigIn();
static Bool ConfigOut();
static Bool y_axis_inversion;

/*
* Global managers. These extern variables are declared in their respective
* header files so they can be accessed anywhere in the project.
*/
MemoryManager* p_memory_manager = nullptr;
FrameTimer* p_frame_timer = nullptr;
GameManager* p_game_manager = nullptr;

SIK::ImGuiWindow* p_imgui_window = nullptr;
InputManager* p_input_manager = nullptr;
GraphicsManager* p_graphics_manager = nullptr;
ResourceManager* p_resource_manager = nullptr;
GameObjectManager* p_game_obj_manager = nullptr;
AudioManager* p_audio_manager = nullptr;
WorldEditor* p_world_editor = nullptr;
PhysicsManager* p_physics_manager = nullptr;
FrameRateManager* p_frame_rate_manager = nullptr;
Factory* p_factory = nullptr;
ParticleSystem* p_particle_system = nullptr;
ScriptingManager* p_scripting_manager = nullptr;
GUIObjectManager* p_gui_object_manager = nullptr;
GameStateManager* p_gamestate_manager = nullptr;

#ifdef STR_DEBUG
StringDictionary* p_dbg_string_dictionary = nullptr;
#endif

/*
* Creates all global managers using the provided allocator.
* Returns : void
*/
void CreateManagersUsingAllocator(PolymorphicAllocator alloc) {
#ifdef STR_DEBUG
    p_dbg_string_dictionary = alloc.new_object<StringDictionary>();
#endif
    p_frame_timer = alloc.new_object<FrameTimer>();
    p_frame_rate_manager = alloc.new_object<FrameRateManager>();
    p_game_manager = alloc.new_object<GameManager>();
    p_input_manager = alloc.new_object<InputManager>();
    p_audio_manager = alloc.new_object<AudioManager>();
    p_graphics_manager = alloc.new_object<GraphicsManager>();
    p_resource_manager = alloc.new_object<ResourceManager>();
    p_game_obj_manager = alloc.new_object<GameObjectManager>();
    p_world_editor = alloc.new_object<WorldEditor>();
    p_physics_manager = alloc.new_object<PhysicsManager>();
    p_factory = alloc.new_object<Factory>();
    p_particle_system = alloc.new_object<ParticleSystem>();
    p_imgui_window = alloc.new_object<SIK::ImGuiWindow>();
    p_scripting_manager = alloc.new_object<ScriptingManager>();
    p_gui_object_manager = alloc.new_object<GUIObjectManager>();
    p_gamestate_manager = alloc.new_object<GameStateManager>();
}


/*
* Destroys all global managers. Provided allocator must match the allocator
* used to create the managers with CreateManagersUsingAllocator.
* Returns : void
*/
void DestroyManagersCreatedByAllocator(PolymorphicAllocator alloc) {
    alloc.delete_object(p_gui_object_manager);
    p_gui_object_manager = nullptr;

    alloc.delete_object(p_scripting_manager);
    p_scripting_manager = nullptr;

    alloc.delete_object(p_imgui_window);
    p_imgui_window = nullptr; 
    
    alloc.delete_object(p_particle_system);
    p_particle_system = nullptr;

    alloc.delete_object(p_game_obj_manager);
    p_game_obj_manager = nullptr;

    alloc.delete_object(p_factory);
    p_factory = nullptr;

    alloc.delete_object(p_world_editor);
    p_world_editor = nullptr;
    
    alloc.delete_object(p_physics_manager);
    p_physics_manager = nullptr;
    
    alloc.delete_object(p_resource_manager);
    p_resource_manager = nullptr;

    alloc.delete_object(p_graphics_manager);
    p_graphics_manager = nullptr;
    
    alloc.delete_object(p_audio_manager);
    p_audio_manager = nullptr;

    alloc.delete_object(p_input_manager);
    p_input_manager = nullptr;

    alloc.delete_object(p_game_manager);
    p_game_manager = nullptr;

    alloc.delete_object(p_frame_rate_manager);
    p_frame_rate_manager = nullptr;

    alloc.delete_object(p_frame_timer);
    p_frame_timer = nullptr;

    alloc.delete_object(p_gamestate_manager);
    p_gamestate_manager = nullptr;


#ifdef STR_DEBUG
    alloc.delete_object(p_dbg_string_dictionary);
    p_dbg_string_dictionary = nullptr;
#endif
}

/*
* Engine intialization function.
* Starts up all the required systems for the engine to run.
* Retuns : bool - True if successful
*/
bool EngineInit() {
#ifdef MEM_DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    noisy_allocator = std::make_unique<DebugMemoryResource>(
        "New-Delete Alloc",
        DebugMemoryResource::Flag::TrackOnly,
        std::pmr::new_delete_resource());
    std::pmr::set_default_resource(noisy_allocator.get());
#endif

    //Initialize LogManager
    LogManager::Init();

    //Initialize memory manager
    PolymorphicAllocator default_allocator = MemoryManager::GetDefaultAllocator();
    p_memory_manager = default_allocator.new_object<MemoryManager>();
    // Maybe set the current allocator to something different for
    // the rest of managers

    //Construct managers
    PolymorphicAllocator managers_allocator = p_memory_manager->GetCurrentAllocator();
    CreateManagersUsingAllocator(managers_allocator);

    // Engine Init
    bool status = p_graphics_manager->InitWindow();
    SIK_ASSERT(status, "Failed to initialize window");

    p_resource_manager->InitDefaultAssets();
    p_resource_manager->BatchLoadAssets("StartupAssets.json");
    
    p_graphics_manager->InitShaders();
    p_imgui_window->Init();

    // Timer initialization
    p_frame_timer->SetFixedDeltaTime(16ms); // fixed_delta_time: time per physics substep

    p_particle_system->Init();

    if (ConfigIn()) {
        SIK_INFO("Configuration set from file SIK_CONFIG.json");
    }

    p_graphics_manager->Setup();

    return status;
}


/*
* Engine cleanup function.
* Starts up all the required systems for the engine to run.
* Retuns : void
*/
void EngineCleanup() {
    
    if (ConfigOut()) {
        SIK_INFO("Configuration saved to file SIK_CONFIG.json");
    }
    
    // Destroy imgui window. 
    // Remove this line after adding it to destructor
    p_imgui_window->Destroy();

    // Destroy managers
    PolymorphicAllocator managers_allocator = p_memory_manager->GetCurrentAllocator();
    DestroyManagersCreatedByAllocator(managers_allocator);

    // Destroy memory manager
    PolymorphicAllocator default_allocator = MemoryManager::GetDefaultAllocator();
    default_allocator.delete_object(p_memory_manager);
    p_memory_manager = nullptr;

#ifdef MEM_DEBUG
    std::pmr::set_default_resource(nullptr);
    noisy_allocator.reset();
#endif

    SDL_Quit();

    // Shutdown spdlog
    LogManager::Destroy();
}

int EngineMain(int argc, char* args[], bool run_prototypes) {    

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            if (std::strcmp(args[i], "debug-draw") == 0 ||
                std::strcmp(args[i], "d") == 0) {
                GraphicsManager::EnableDebugDrawing();
            }
        }
    }

    RenderCam engine_camera(p_graphics_manager->GetWindowWidth(),
                            p_graphics_manager->GetWindowHeight());
    engine_camera.SetPosition(Vec3(0, 0, 10));
    engine_camera.SetYAxisInversion(y_axis_inversion);
    p_graphics_manager->SetPActiveCam(&engine_camera);
    p_game_manager->SetEngineCam(&engine_camera);


    // Uncomment this block if you want to process all assets from the start up 
    // manifest before entering the main loop
    {
        SIK_TIMER("Waiting for assets to process...");
        Uint32 pre_loaded_asset_count = p_resource_manager->ProcessAllAssets();
        SIK_INFO("{} assets processed before entering main loop", pre_loaded_asset_count);
    }

    // Load the game and run it
    p_game_manager->SetRunArgs(argc, args);
    p_game_manager->LoadDlls();
#ifndef _PROTOTYPE
    p_game_manager->RunGame();
#endif // !_PROTOTYPE


    {   
        auto accumulator{ 0ns };               // counts lag btwn sim and real time
        auto prev = FrameTimer::Now();         // get time before main loop starts
        auto fixed_time_step = p_frame_timer->GetFixedDeltaTime();
        Float32 const fixed_dt = FrameTimer::ToSeconds<Float32>(fixed_time_step);

        // The below line shows an example of how to create a new state
        // To test it uncomment the below two lines and see the console
        // Press L to change between states
        //p_gamestate_manager->PushState(new PlayState("Play State"));
        //p_gamestate_manager->PushState(new MenuState("Menu State"));

        //Game loop
        while (p_game_manager->Status()) {
            // Frame Start
            auto current = FrameTimer::Now();
            auto elapsed = current - prev;
            prev = current;
            accumulator += elapsed;
            Float32 dt = FrameTimer::ToSeconds<Float32>(elapsed);

            // Asset Processing
            Uint32 assets_processed = p_resource_manager->ProcessAssetsFor(2ms);
            if (assets_processed > 0) {
                SIK_INFO("{} assets processed this frame", assets_processed);
            }
            
            // Input and Audio
            p_input_manager->Update();
            p_audio_manager->Update();

            // Physics Update
            while (accumulator >= fixed_time_step) {
                p_physics_manager->Update(fixed_dt);
                p_gamestate_manager->FixedUpdate(fixed_dt);
                accumulator -= fixed_time_step;
            }
            p_physics_manager->Extrapolate(FrameTimer::ToSeconds<Float32>(accumulator)/* / fixed_dt*/);
            
            // Game Update
            p_game_manager->Update(dt);
#ifdef _PROTOTYPE
            p_gui_object_manager->Update();
            p_game_obj_manager->Update(dt);
#endif // _PROTOTYPE
            p_gamestate_manager->Update(dt);

            // Rendering
            p_particle_system->Update(dt);
            p_graphics_manager->Update(dt);
            engine_camera.Update(dt);

            p_graphics_manager->RenderScene();
#ifdef _ENABLE_EDITOR
            p_imgui_window->BeginRender();
            p_imgui_window->Render();
#endif
            p_graphics_manager->SwapBuffers();

            // Frame End
            p_game_obj_manager->CleanupDeletedObjects();
            p_frame_rate_manager->FrameEnd(dt);
        }
    }

    // Unload DLL object
    p_game_manager->UnloadDlls();

    y_axis_inversion = engine_camera.GetYAxisInversion();
    return 0;
}


#include <rapidjson.h>

namespace fs = std::filesystem;
namespace rj = rapidjson;


// TODO: (Dylan) add support for window size/Fullscreen settings
static Bool ConfigIn() {
    static char buf[1024];
    static const fs::path filepath = fs::current_path() / "SIK_CONFIG.json";

    // Open our file. Need to use C-style here because RapidJSON needs a FILE*.
    FILE* fp = std::fopen(filepath.string().c_str(), "rb");
    if (fp == nullptr) {
        SIK_WARN("Configuration file failed to open. Using default configuration.");
        return false;
    }

    // Use RapidJSON to read and parse the file as a rapidjson::Document, then close the file
    rj::Document config{};
    rapidjson::FileReadStream input_stream{ fp, buf, 1024 };
    config.ParseStream(input_stream);
    std::fclose(fp);

    if (not config.IsObject()) {
        SIK_WARN("Configuration file was invalid. Using default configuration.");
        return false;
    }

    auto it = config.MemberBegin();


#define READ_CONFIG_VALUE(member_name, expected_type, set_value_code) \
        it = config.FindMember(member_name);\
        if (it != config.MemberEnd()) {\
            if (it->value.Is##expected_type##()) {\
                set_value_code\
            }\
            else {\
                SIK_WARN("{} must be a {}. Using default value.", member_name, #expected_type); \
            }\
        }    

        /*READ_CONFIG_VALUE("SFX Volume", Float,
            p_audio_manager->SetVolume(it->value.GetFloat(), p_audio_manager->sfx_chanel_group);)

        READ_CONFIG_VALUE("Music Volume", Float,
            p_audio_manager->SetVolume(it->value.GetFloat(), p_audio_manager->menu_stuff_chanel_group);)

        READ_CONFIG_VALUE("Background Volume", Float,
            p_audio_manager->SetVolume(it->value.GetFloat(), p_audio_manager->background_music_chanel_group);)*/

        READ_CONFIG_VALUE("Invert Camera Y-Axis", Bool, y_axis_inversion = it->value.GetBool();)

        READ_CONFIG_VALUE("Enable Debug Drawing", Bool,
            if (it->value.GetBool()) { GraphicsManager::EnableDebugDrawing(); })

#undef READ_CONFIG_VALUE

        return true;
}

static Bool ConfigOut() {
    static char buf[1024];

    rj::Document doc{};
    doc.SetObject();

    rj::Value key{};

    /*key.SetString("SFX Volume");
    doc.AddMember(key, p_audio_manager->GetVolume(p_audio_manager->sfx_chanel_group), doc.GetAllocator());

    key.SetString("Music Volume");
    doc.AddMember(key, p_audio_manager->GetVolume(p_audio_manager->menu_stuff_chanel_group), doc.GetAllocator());
    
    key.SetString("Background Volume");
    doc.AddMember(key, p_audio_manager->GetVolume(p_audio_manager->background_music_chanel_group), doc.GetAllocator());*/
    
    key.SetString("Invert Camera Y-Axis");
    doc.AddMember(key, y_axis_inversion, doc.GetAllocator());

    key.SetString("Enable Debug Drawing");
    doc.AddMember(key, GraphicsManager::IsDebugDrawingEnabled(), doc.GetAllocator());


    FILE* fp = std::fopen("SIK_CONFIG.json", "w");
    if (fp == nullptr) {
        SIK_ERROR("Failed to open or create SIK_CONFIG.json. Configuration not written.");
        return false;
    }

    rapidjson::FileWriteStream stream{ fp, buf, 1024 };
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer{ stream };

    Bool result = doc.Accept(writer);

    std::fclose(fp);
    return result;
}