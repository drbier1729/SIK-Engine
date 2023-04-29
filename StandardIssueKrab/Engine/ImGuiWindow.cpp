#include "stdafx.h"

#include "PerformanceManager.h"
#include "WorldEditor.h"
#include "AudioManager.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <algorithm>
#include <commdlg.h>

#include "MemoryResources.h"
#include "GraphicsManager.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "Factory.h"
#include "GameManager.h"
#include "FrameRateManager.h"
#include "RenderCam.h"
#include "GraphicsManager.h"
#include "PhysicsManager.h"
#include "GameObject.h"
#include "ImGuiWindow.h"
#include "Behaviour.h"
#include "ScriptingManager.h"
#include "Component.h"

using namespace SIK;

static bool show_audio_functions = false;
static bool show_performance = false;
static bool show_graphics_functions = false;
static bool show_camera_functions = false;
static bool show_scene_editor = false;
static bool show_timer_tool = false;

#ifdef GAME_COMPONENT_LIST_FILE
    #define REGISTER_COMPONENT(component) class component;
    #include GAME_COMPONENT_LIST_FILE
    #undef REGISTER_COMPONENT
#endif


static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

struct InputTextWraper
{
    static int ResizeCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            String* dynamic_str = (String*)data->UserData;
            IM_ASSERT(dynamic_str->c_str() == data->Buf);
            dynamic_str->resize(data->BufSize + 1); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
            data->Buf = (char*)dynamic_str->c_str();
        }
        return 0;
    }

    static bool InputText(const char* label, String& dynamic_str, ImGuiInputTextFlags flags = 0)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        return ImGui::InputText(label, &dynamic_str[0], (size_t)dynamic_str.size(), flags | ImGuiInputTextFlags_CallbackResize, InputTextWraper::ResizeCallback, (void*)&dynamic_str);
    }
};

ImGuiWindow::ImGuiWindow() : current_filename(nullptr) {}
ImGuiWindow::~ImGuiWindow() {
    p_memory_manager->GetCurrentAllocator().deallocate(reinterpret_cast < Byte*>(current_filename), 256);
}

void ImGuiWindow::Init() {
    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();

    SDL_Window* window = p_graphics_manager->GetPWindow();
    SDL_GLContext context = p_graphics_manager->GetGLContext();


    ImGui_ImplSDL2_InitForOpenGL(window, &context);
    ImGui_ImplOpenGL3_Init("#version 330"); 

    total_time = 60;

    current_filename = reinterpret_cast<char*>(p_memory_manager->GetCurrentAllocator().allocate(256));

    // create a vector of scripts
    for (const std::filesystem::directory_entry& entry : 
        std::filesystem::directory_iterator(std::filesystem::current_path() / ".." / "StandardIssueKrab" / "Engine" / "Assets" / "Scripts")) {
        ScriptNames.push_back((String)entry.path().filename().string());
    }

    // create a vector of archetypes
    for (const std::filesystem::directory_entry& entry : 
        std::filesystem::directory_iterator(std::filesystem::current_path() / ".." / "StandardIssueKrab" / "Engine" / "Assets" / "Archetypes")) {
        ArchNames.push_back((String)entry.path().filename().string());
    }

    // Engine side component
    CompNames.push_back("RigidBody");
    CompNames.push_back("Behaviour");

    // Game side component
#ifdef GAME_COMPONENT_LIST_FILE
#define REGISTER_COMPONENT(component) CompNames.push_back(#component);
#include GAME_COMPONENT_LIST_FILE
#undef REGISTER_COMPONENT
#endif
}

void ImGuiWindow::HandleSDLEvent(SDL_Event& e) {
    
    ImGui_ImplSDL2_ProcessEvent(&e);
}

void ImGuiWindow::BeginRender() {
    
    ImGui_ImplOpenGL3_NewFrame(); 
    ImGui_ImplSDL2_NewFrame();    
    ImGui::NewFrame();
}

void ImGuiWindow::ShowMenu() {
    if(ImGui::BeginMainMenuBar()) 
    {   
        current_filename[0] = '\0';
        std::strcat(current_filename, "File: ");
        const char* basename = std::strrchr(p_world_editor->GetSceneFileName(), '\\');
        std::strcat(current_filename, basename == nullptr ? "" : basename + 1);

        if (ImGui::BeginMenu(current_filename)) {
            if (ImGui::MenuItem("Open")) 
            { 
                OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                wchar_t szFile[_MAX_PATH];
                LPCWSTR szExt = L"*.json";
                const char* filename = p_world_editor->GetSceneFileName();

                // SDL_syswm.h query HWND from SDL2 window
                SDL_SysWMinfo wmInfo;
                SDL_VERSION(&wmInfo.version);
                SDL_GetWindowWMInfo(p_graphics_manager->GetPWindow(), &wmInfo);
                HWND hwnd = wmInfo.info.win.window;
                ofn.hwndOwner = hwnd;
                
                mbstowcs(szFile, filename, strlen(filename) + 1);
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
                ofn.lpstrFilter = ofn.lpstrDefExt = szExt;
                ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

                if (GetOpenFileName(&ofn))
                {
                    char file[_MAX_PATH];
                    wcstombs(file, ofn.lpstrFile, lstrlenW(ofn.lpstrFile));

                    String str;
                    for (int i = 0; i < lstrlenW(ofn.lpstrFile); ++i) {
                        str.push_back(file[i]);
                    }

                    p_world_editor->SetOpenFileName(str.c_str());
                }
            }

            if (ImGui::MenuItem("Save")) 
            {
                p_world_editor->SaveScene();
            }

            ImGui::EndMenu();            
        }

#ifdef _PROTOTYPE
        if (p_game_manager->IsTestRunning())
        {
            if (ImGui::BeginMenu("Test - Running"))
            {
                if (ImGui::MenuItem("End Running Test"))
                    p_game_manager->EndRunningTest();
                ImGui::EndMenu();
            }
        }
        else
        {
            if (ImGui::BeginMenu("Test"))
            {
                if (TestNames.size() == 0) TestNames = p_game_manager->getTestNames();
                for (int i = 0; i < TestNames.size(); ++i) {

                    if (ImGui::MenuItem(TestNames[i].c_str()))
                    {
                        p_game_manager->LaunchTestFromImGui(i);
                    }

                }
                ImGui::EndMenu();
            }
        }
#endif // _PROTOTYPE

        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("Audio",       NULL, &show_audio_functions);
            ImGui::MenuItem("Graphics",    NULL, &show_graphics_functions);
            ImGui::MenuItem("Performance", NULL, &show_performance);
            ImGui::MenuItem("Camera",      NULL, &show_camera_functions);
            ImGui::MenuItem("SceneEditor", NULL, &show_scene_editor);
            ImGui::MenuItem("Timer",       NULL, &show_timer_tool);
            ImGui::EndMenu();
        }

#ifdef _PROTOTYPE
        ShowPrototypeMenuBar();
#endif // _PROTOTYPE

        ImGui::EndMainMenuBar();
    }
}

template <class C>
Bool ShowComponent(const char* name, C* p_comp, bool const& pop = true) {

    // null pointer to component
    if (p_comp == nullptr) return false;

    Bool open = false;
    open = ImGui::TreeNode(name);
    if (open) {

        Reflector::Class const* object_info = Reflector::GetClass<C>();
        for (auto const& field : object_info->fields) {
            if (field.type == nullptr) {
                break;
            }

            Byte* source = reinterpret_cast<Byte*>(p_comp) + field.offset;
            switch (field.type->enum_name) {
            case Reflector::TypeName::Vec3: {
                Vec3  destination(0.0);

                // get value from components
                memcpy(&destination, source, field.type->size);

                // show on editor
                ImGui::DragFloat3(field.name, &destination[0]);

                // save back to component
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Vec4: {
                Vec4  destination(0.0);

                memcpy(&destination, source, field.type->size);
                ImGui::InputFloat4(field.name, &destination[0]);
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Quat: {
                Quat destination = Quat(1, 0, 0, 0);

                memcpy(&destination, source, field.type->size);
                static Vec3 euler(0.0f, 0.0f, 0.0f);
                Vec3 init_val = euler;
                ImGui::DragFloat3(field.name, &euler[0]);
            
                if(glm::dot(euler,init_val) > 0.99999 ) {
                    destination = glm::rotate(Quat(1, 0, 0, 0), glm::radians(euler[0]), Vec3(1, 0, 0)) *
                                  glm::rotate(Quat(1, 0, 0, 0), glm::radians(euler[1]), Vec3(0, 1, 0)) *
                                  glm::rotate(Quat(1, 0, 0, 0), glm::radians(euler[2]), Vec3(0, 0, 1));
                }
                
                memcpy(source, &destination, field.type->size);
                //ImGui::SameLine(); HelpMarker("Quat is read-only.");
                break;
            }
            case Reflector::TypeName::Float32: {
                Float32 destination = 0.0f;

                memcpy(&destination, source, field.type->size);
                ImGui::DragFloat(field.name, &destination, 1.0f, std::numeric_limits<Float32>::min(), std::numeric_limits<Float32>::max());
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Float64: {
                float destination = 0.0f;

                memcpy(&destination, source, field.type->size);
                ImGui::DragFloat(field.name, &destination, 1.0f, std::numeric_limits<Float32>::min(), std::numeric_limits<Float32>::max());
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Int8:
            case Reflector::TypeName::Int16:
            case Reflector::TypeName::Int32: {
                Int32 destination = 0;

                memcpy(&destination, source, field.type->size);
                ImGui::DragInt(field.name, &destination, 1.0f, std::numeric_limits<Int32>::min(), std::numeric_limits<Int32>::max());
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Int64: {
                int destination = 0;

                memcpy(&destination, source, field.type->size);
                ImGui::DragInt(field.name, &destination, 1.0f, std::numeric_limits<Int32>::min(), std::numeric_limits<Int32>::max());
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Uint8:
            case Reflector::TypeName::Uint16:
            case Reflector::TypeName::Uint32: {
                Int32 destination = 0;

                memcpy(&destination, source, field.type->size);
                ImGui::DragInt(field.name, &destination, 1.0f, std::numeric_limits<Uint32>::min(), std::numeric_limits<Uint32>::max());
                memcpy(source, &destination, field.type->size);
                break;
            }
            case Reflector::TypeName::Bool: {
                Bool destination = false;

                memcpy(&destination, source, field.type->size);
                ImGui::Checkbox(field.name, &destination);
                memcpy(source, &destination, field.type->size);
                break;
            }
            }

        }

        if (pop) ImGui::TreePop();
    }

    return open;
}

void ImGuiWindow::ShowObjectComponent(GameObject* GameObject)
{   
    // Set Name
    String go_name = GameObject->GetName();
    if (go_name.empty()) go_name.push_back(0);
    InputTextWraper::InputText("Name", go_name);
    GameObject->SetName(go_name.c_str());
    

    // Transform
    Transform* transform = GameObject->HasComponent<Transform>();
    ShowComponent("Transform", transform);

    // RigidBody
    RigidBody* rigidbody = GameObject->HasComponent<RigidBody>();
    Bool open = ShowComponent("RigidBody", rigidbody, false);
    if (rigidbody != nullptr && open) {
            
        // mass
        ImGui::DragFloat("mass", &rigidbody->motion_props->mass, 1.0f, std::numeric_limits<Float32>::min(), std::numeric_limits<Float32>::max());
            
        // gravity
        ImGui::DragFloat("gravity", &rigidbody->motion_props->gravity_scale, 1.0f, std::numeric_limits<Float32>::min(), std::numeric_limits<Float32>::max());
        
        ImGui::TreePop();
    }

    Behaviour* bh = GameObject->HasComponent<Behaviour>();
    if (bh != nullptr) {
        if (ImGui::TreeNode("Behaviour")) {
            auto& scripts = bh->scripts;

            // flags for combo configuration
            static ImGuiComboFlags flags = 0;

            // Remove scripts
            static int Remove_current_idx = 0;
            const int remaining_scripts = scripts.size();
            const char* Remove_preview_value = Remove_current_idx < remaining_scripts ? scripts[Remove_current_idx].script_name : "";
            if (ImGui::BeginCombo("Scripts-to-Remove", Remove_preview_value, flags))
            {
                for (int n = 0; n < scripts.size(); n++)
                {
                    const bool is_selected = (Remove_current_idx == n);
                    if (ImGui::Selectable(scripts[n].script_name, is_selected))
                        Remove_current_idx = n;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Remove Script") && remaining_scripts > 0) {
                bh->RemoveScript(scripts[Remove_current_idx].script_name);
            }


            // Add scripts
            static int Add_current_idx = 0;
            const char* Add_preview_value = ScriptNames[Add_current_idx].c_str();
            if (ImGui::BeginCombo("Scripts-to-Add", Add_preview_value, flags))
            {
                for (int n = 0; n < ScriptNames.size(); n++)
                {
                    const bool is_selected = (Add_current_idx == n);
                    if (ImGui::Selectable(ScriptNames[n].c_str(), is_selected))
                        Add_current_idx = n;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::Button("Add Script")) {
                auto it = bh->scripts.begin();

                // compare scripts
                for (; it != bh->scripts.end(); ++it) {
                    if (std::strcmp(it->script_name, ScriptNames[Add_current_idx].c_str()) == 0) break;
                }

                // prevent adding the same script
                if (it == bh->scripts.end()) {
                    bh->AddScript(ScriptNames[Add_current_idx].c_str());
                    bh->LoadSingleScript(ScriptNames[Add_current_idx].c_str());
                }
            }
            ImGui::TreePop();
        }
    }

#ifndef _PROTOTYPE
    for (auto comp = GameObject->GetComponentArray().begin(); comp != GameObject->GetComponentArray().end(); ++comp) {
        Component* p_comp = comp->get();
        Component::Type type = p_comp->GetType();
            
        switch (type) { 
#define REGISTER_COMPONENT(component) \
        case Component::Type::##component: {\
            component* c = reinterpret_cast<component*>(p_comp);\
            ShowComponent(p_comp->GetName(), c);\
            break;\
        };
#include GAME_COMPONENT_LIST_FILE
#undef REGISTER_COMPONENT
        }
    }
#endif // _PROTOTYPE


    static int current_idx = 0;
    const int comp_names = CompNames.size();
    const char* preview_value = CompNames[current_idx].c_str();
    if (ImGui::BeginCombo("Components", preview_value, 0))
    {
        for (int n = 0; n < CompNames.size(); n++)
        {
            const bool is_selected = (current_idx == n);
            if (ImGui::Selectable(CompNames[n].c_str(), is_selected))
                current_idx = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
        

    const char* curr_comp_name = CompNames[current_idx].c_str();
    if (ImGui::Button("Add Component")) {
            
        auto comp = GameObject->GetComponentArray().begin();

        // find existed comp
        for (; comp != GameObject->GetComponentArray().end(); ++comp) {
            if (std::strcmp(comp->get()->GetName(), curr_comp_name) == 0) break;
        }

        // check if adding existed comp
        if (comp == GameObject->GetComponentArray().end()) {
                
            if (std::strcmp(curr_comp_name, "RigidBody") == 0) {
            }
            else if (std::strcmp(curr_comp_name, "Behaviour") == 0) {
                Behaviour* new_bh = p_scripting_manager->CreateBehaviour();
                GameObject->AddComponent(new_bh);
                p_world_editor->AddComp(GameObject, ToStringID(curr_comp_name));
            }
                
            // construct component
            Component* p_comp = p_factory->GetGameSideComp(ToStringID(curr_comp_name));
            if (p_comp != nullptr) {
                GameObject->AddComponent(p_comp);

                // add to hashmap for saving
                p_world_editor->AddComp(GameObject, ToStringID(curr_comp_name));
            }
        }
    }

    if (ImGui::Button("Remove Component")) {

        // Engine side component
        if (std::strcmp(curr_comp_name, "RigidBody") == 0) {
            RigidBody* rb = GameObject->HasComponent<RigidBody>();
            if(rb != nullptr) p_physics_manager->RemoveRigidBody(rb);
        }
        else if (std::strcmp(curr_comp_name, "Behaviour") == 0) {
            Behaviour* bh = GameObject->HasComponent<Behaviour>();
            if (bh != nullptr) {
                p_scripting_manager->DeleteBehaviour(bh);
                p_world_editor->RemoveComp(GameObject, ToStringID(curr_comp_name));
            }
        }

        // Game side component
        for (auto comp = GameObject->GetComponentArray().begin(); comp != GameObject->GetComponentArray().end(); ++comp) {

            // find comp
            if (std::strcmp(comp->get()->GetName(), curr_comp_name) != 0) continue;               

            // delete comp from array
            comp->release();
            GameObject->GetComponentArray().erase(comp);
                
            // add to hashmap for removal
            p_world_editor->RemoveComp(GameObject, ToStringID(curr_comp_name));
            break;
        }
    }
}

void ImGuiWindow::ShowSceneEditor() {
    static GameObject* selected = nullptr;
    static GameObject* hovered = nullptr;
    static Bool editMode = false;
    // Hacked in way to select an object in the world using the mouse
    Ivec2 const cursor_pos = p_input_manager->GetMousePos();
    auto [p, d] = p_graphics_manager->GetPActiveCam()->ScreenPointToRay(cursor_pos);
    auto cast_result = p_physics_manager->RayCast({p, d});

    hovered = cast_result.object;
    if (p_input_manager->IsMouseButtonTriggered(SDL_MOUSE_BUTTON_LEFT)){ 
        selected = hovered ? hovered : selected;
    }

    // get object list                
    auto* pool = &p_game_obj_manager->GetGameObjectContainer();

    if (!ImGui::Begin("Scene editor"))
    {
        ImGui::End();
        return;
    }

    const char* hovered_name = hovered ? hovered->GetName().c_str() : "<NONE>";
    const char* selected_name = selected ? selected->GetName().c_str() : "<NONE>";
    ImGui::Text("Hovered Object: %s", hovered_name);
    ImGui::Text("Selected Object: %s", selected_name);
    ImGui::Text("Edit Mode: %s", editMode ? "On" : "Off");
    if (editMode) {
        static ImGuiComboFlags flags = 0;

        // Remove scripts
        static int current_arch_idx = 0;
        const char* preview_value = current_arch_idx < (int)ArchNames.size() ? ArchNames[current_arch_idx].c_str() : "";
        if (ImGui::BeginCombo("Archetypes", preview_value, flags))
        {
            for (int n = 0; n < ArchNames.size(); n++)
            {
                const bool is_selected = (current_arch_idx == n);
                if (ImGui::Selectable(ArchNames[n].c_str(), is_selected))
                    current_arch_idx = n;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        static int place_mode = 0, quantity = 1, height = 1;
        static float dist = 2.5f;
        ImGui::SliderInt("mode", &place_mode, 0, 4, "%d");
        ImGui::InputFloat("dist", &dist, 0.1f, 0.1f, "%.1f");
        ImGui::InputInt("quantity", &quantity);
        ImGui::InputInt("height", &height);

        if (p_input_manager->IsMouseButtonTriggered(SDL_MOUSE_BUTTON_LEFT)) {

            // create new go
            const char* archetype = preview_value;
            int inner_mode = place_mode == 4 ? 0 : place_mode;
            int inner_quantity = quantity;
            Vec3 point = cast_result.info.point;
            point.x = floorf(point.x);
            point.z = floorf(point.z);
            switch (inner_mode) {
            case 0:
                point.z -= dist;
                break;
            case 1:
                point.x += dist;
                break;
            case 2:
                point.z += dist;
                break;
            case 3:
                point.x -= dist;
                break;
            case 4:
                point.z -= dist;
                break;
            }

            do {

                for (int i = 0; i < inner_quantity; ++i) {

                    switch (inner_mode) {
                    case 0:
                        point.z += dist;
                        break;
                    case 1:
                        point.x -= dist;
                        break;
                    case 2:
                        point.z -= dist;
                        break;
                    case 3:
                        point.x += dist;
                        break;
                    }

                    // for cubes to stack up
                    for (int h = 0; h < height; h++) {
                        GameObject* new_go = p_factory->BuildGameObject(archetype);

                        // set name for new go
                        // to prevent naming conflict, add number to the end of the name
                        String new_name = "";
                        std::strcat(&(new_name[0]), new_go->GetName().c_str());
                        std::strcat(&(new_name[0]), "_");
                        std::strcat(&(new_name[0]), std::to_string(pool->size()).c_str());
                        new_go->SetName(new_name.c_str());



                        // place object at the mouse position
                        RigidBody* rb = new_go->HasComponent<RigidBody>();
                        Transform* tr = new_go->HasComponent<Transform>();
                        float new_y = point.y + static_cast<float>(h) * dist;
                        if (rb != nullptr) {
                            rb->position = Vec3(point.x, new_y, point.z);
                            tr->SetPosition(point.x, new_y , point.z);
                        }
                        else {
                            tr->SetPosition(point.x, new_y, point.z);
                        }

                        // Add object to the save list
                        p_world_editor->SaveNewGameObject(new_go, ToStringID(archetype));

                    }
                }

                inner_mode++;
                inner_quantity = quantity - 1;
                if (inner_mode == 3) inner_quantity--;

            } while (inner_mode < place_mode);
        }
    }
    
    ImGui::Separator();

    for (auto r = pool->all(); not r.is_empty(); r.pop_front()) {
        if (ImGui::Selectable(r.front().GetName().c_str(), false)) {
            selected = &r.front();
        }
    }

    if (selected) {
        if (!ImGui::Begin("Components editor")) {
            ImGui::End();
            return;
        }
        
        ImGui::Text("Name:");
        ImGui::SameLine(); ImGui::Text(selected_name);
        ShowObjectComponent(selected);
        ImGui::End();
    }
    
    
    if (p_input_manager->IsKeyReleased(SDL_SCANCODE_F2)) editMode = !editMode;

    ImGui::End();
}

void ImGuiWindow::ShowAudioFunctions() {
    ImGui::Begin("Audio Functions");
        // Handling audio pausing and resuming
        if (ImGui::BeginMenu("Play")) {

            if (ImGui::BeginMenu("Sound Effects")) {

                if (ImGui::MenuItem("SKid Sound")) {
                    p_audio_manager->PlayAudio("SKID"_sid,
                        p_audio_manager->sfx_chanel_group,
                        p_audio_manager->skid_vol,
                        1.5f, false, 0);
                }
                if (ImGui::MenuItem("Turret Impact Sound")) {
                    p_audio_manager->PlayAudio("TURRET_IMP"_sid,
                        p_audio_manager->sfx_chanel_group,
                        p_audio_manager->turret_impact_vol,
                        1.5f,
                        false, 
                        0);
                }
                if (ImGui::MenuItem("Turret Dead Sound")) {
                    p_audio_manager->PlayAudio("TURRET_DES"_sid,
                        p_audio_manager->sfx_chanel_group,
                        p_audio_manager->turret_destroy_vol,
                        1.5f,
                        false,
                        0);
                }
                if (ImGui::MenuItem("Turret Shooting Sound")) {
                    p_audio_manager->PlayAudio("NAIL_SHOOT"_sid,
                        p_audio_manager->sfx_chanel_group,
                        p_audio_manager->turret_shoot_vol,
                        1.0f,
                        false,
                        0);
                }
                if (ImGui::MenuItem("Wooden Box Destroying Sound")) {
                    p_audio_manager->PlayAudio("IMPACT"_sid,
                        p_audio_manager->sfx_chanel_group,
                        p_audio_manager->wood_destroy_vol,
                        1.5,
                        false,
                        0);
                }
               
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }

        /*if (ImGui::BeginMenu("Pause")) {
            
            if (ImGui::MenuItem("Pause Background")) {
                p_audio_manager->Pause(p_audio_manager->background_chanel);
            }

            if (ImGui::MenuItem("Pause Music")) {
                p_audio_manager->Pause(p_audio_manager->music_chanel);
            }

            if (ImGui::MenuItem("Pause Sfx")) {
                p_audio_manager->Pause(p_audio_manager->sfx_chanel);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Unpause")) {
            
            if (ImGui::MenuItem("Unpause Background")) {
                p_audio_manager->Unpause(p_audio_manager->background_chanel);
            }

            if (ImGui::MenuItem("Unpause Music")) {
                p_audio_manager->Unpause(p_audio_manager->music_chanel);
            }

            if (ImGui::MenuItem("Unpause Sfx")) {
                p_audio_manager->Unpause(p_audio_manager->sfx_chanel);
            }

            ImGui::EndMenu();
        }*/

        if (ImGui::BeginMenu("Volume")) {

            if (ImGui::BeginMenu("Sound Effects")) {

                ImGui::SliderFloat("Skid Vol.", &p_audio_manager->skid_vol, 0.0f, 100.0f);
                ImGui::SliderFloat("Turret Destroying Vol", &p_audio_manager->turret_destroy_vol, 0.0f, 100.0f);
                ImGui::SliderFloat("Turret Impact Vol", &p_audio_manager->turret_impact_vol, 0.0f, 100.0f);
                ImGui::SliderFloat("Turret Shooting Vol", &p_audio_manager->turret_shoot_vol, 0.0f, 100.0f);
                ImGui::SliderFloat("Wooden Box Destroying Vol", &p_audio_manager->wood_destroy_vol, 0.0f, 100.0f);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Arena Background")) {

                ImGui::SliderFloat("Construction Arena Vol.", &p_audio_manager->construction_arena_back_vol, 0.0f, 100.0f, "");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
    ImGui::End();
}

void ImGuiWindow::ShowPerformance() {
    static double stopwatch = 0.0;


    ImGui::Begin("Performance Viewer");    

    
    if (stopwatch == 0.0) {        
        stopwatch = ImGui::GetTime();
    }

    if (ImGui::GetTime() - stopwatch > 0.5) {
        fps.push_back(static_cast<Float32>(p_frame_rate_manager->GetFPS()));
        stopwatch = ImGui::GetTime();
    }
   
    if (fps.size() > 10) {
        std::copy(fps.begin() + 1, fps.end(), fps.begin());
        fps.pop_back();
    }
    
    if (fps.size() > 0) {
        ImGui::PlotLines("Frame Times", &fps[0], static_cast<int>(fps.size()), 1, NULL, 0.0f, 120.0f, ImVec2(0, 80.0f));
        ImGui::PlotHistogram("Histogram", &fps[0], static_cast<int>(fps.size()), 0, NULL, 0.0f, 120.0f, ImVec2(0, 80.0f));
    }
    
    Bool* vsync_toggle = &p_graphics_manager->GetVSyncToggle();
    if(ImGui::Checkbox("V-Sync", vsync_toggle))
    {
        p_graphics_manager->ToggleVSync();
    }

    ImGui::End();
}

void ImGuiWindow::ShowGriphicsFunctions() {

    // pointers to toggle
    Bool* skyToggle   = &p_graphics_manager->GetSkyToggle();
    Bool* bgToggle    = &p_graphics_manager->GetBgToggle();
    Bool* bgTexToggle = &p_graphics_manager->GetBgTexToggle();
    Bool* normalToggle = &p_graphics_manager->GetNormalToggle();

    // pointer to msaa toggle
    Bool* msaaToggle = &p_graphics_manager->GetMsaaToggle();

    Bool* wireframeToggle = &p_graphics_manager->GetWireframeToggle();

    Bool* particlesToggle = &p_graphics_manager->GetGeoParticlesToggle();

    // colors for background
    Vec3 *color1 = &p_graphics_manager->GetColor1();
    Vec3 *color2 = &p_graphics_manager->GetColor2();
    float col1[3] = { color1->r, color1->g, color1->b };
    float col2[3] = { color2->r, color2->g, color2->b };

    ImGui::Begin("Graphics Config");        
    if (ImGui::Button("FullScreen")) {
        p_graphics_manager->ToggleFullScreen();
    }

    ImGui::Checkbox("Normals", normalToggle);
        if (ImGui::Checkbox("Sky Dome", skyToggle)) *bgToggle = false;
        if (ImGui::Checkbox("Background", bgToggle)) *skyToggle = false;
        if(*bgToggle) ImGui::Checkbox("Texture", bgTexToggle);
        if (*bgToggle && !(*bgTexToggle)) {
            ImGui::ColorEdit3("color1", col1);
            ImGui::ColorEdit3("color2", col2);
        }

        ImGui::Checkbox("MSAA", msaaToggle);
        ImGui::Checkbox("Wireframe mode", wireframeToggle);
        ImGui::Checkbox("Deferred Shading enabled", &p_graphics_manager->deferred_shading_enabled);
        ImGui::Checkbox("Debug Local Lights", &p_graphics_manager->debug_local_lights);

        ImGui::SliderFloat("Gamma ", &p_graphics_manager->gamma, 1, 20);
        ImGui::SliderFloat("Exposure ", &p_graphics_manager->exposure, 1, 20);

        if (ImGui::BeginMenu("Draw FBOs")) {
            if (ImGui::MenuItem("Disable", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::DISABLE)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::DISABLE;
            }
            if (ImGui::MenuItem("Draw WorldPos Buffer", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::WORLD_POS)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::WORLD_POS;
            }
            if (ImGui::MenuItem("Draw Normal Buffer", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::NORMALS)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::NORMALS;
            }
            if (ImGui::MenuItem("Draw Diffuse Color Buffer", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::DIFFUSE)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::DIFFUSE;
            }
            if (ImGui::MenuItem("Draw Specular Color Buffer", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::SPECULAR)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::SPECULAR;
            }
            if (ImGui::MenuItem("Draw Shadow Buffer", "", 
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::SHADOW)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::SHADOW;
            }
            if (ImGui::MenuItem("Draw Bloom Downsample", "",
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::BLOOM_DOWNSAMPLE)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::BLOOM_DOWNSAMPLE;
            }
            if (ImGui::MenuItem("Draw Bloom Upsample", "",
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::BLOOM_UPSAMPLE)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::BLOOM_UPSAMPLE;
            }

            if (ImGui::MenuItem("Draw Emissions Buffer", "",
                p_graphics_manager->draw_buffer == GraphicsManager::DrawBuffer::EMISSION)) {
                p_graphics_manager->draw_buffer = GraphicsManager::DrawBuffer::EMISSION;
            }
            ImGui::EndMenu();
        }

        ImGui::Checkbox("Geometric particles", particlesToggle);

        ImGui::Checkbox("Bloom Enabled", &p_graphics_manager->bloom_enabled);
        if (p_graphics_manager->bloom_enabled) {
            ImGui::SliderFloat("Bloom Threshold", &p_graphics_manager->bloom_threshold, 0, 3);
            ImGui::SliderFloat("Bloom Factor", &p_graphics_manager->bloom_factor, 0, 1);
            ImGui::SliderInt("Bloom Downsample passes", 
                &p_graphics_manager->bloom_downsample_passes, 2, 8);
            ImGui::SliderInt("Bloom Mip level", 
                &p_graphics_manager->bloom_mip_level, 0, 
                p_graphics_manager->bloom_downsample_passes);
        }
    ImGui::End();

    *color1 = Vec3(col1[0], col1[1], col1[2]);
    *color2 = Vec3(col2[0], col2[1], col2[2]);
}

void ImGuiWindow::ShowCameraControls() {
    RenderCam* active_cam = p_graphics_manager->GetPActiveCam();
    bool* invertY = &active_cam->GetYAxisInversion();

    ImGui::Begin("Camera Config");
    ImGui::Checkbox("Invert Camera Y", invertY);
    
    
    const char* control_items[] = { "NONE", "VIEWER", "FIRST_PERSON", "THIRD_PERSON", "ISOMETRIC", "TOPDOWN"};
    static int current_control_index = 1;
    static ControlMode cam_control_mode = ControlMode::VIEWER;
    const char* control_label = control_items[current_control_index];

    if (ImGui::BeginCombo("Controller Mode", control_label))
    {
        for (int n = 0; n < IM_ARRAYSIZE(control_items); n++)
        {
            bool is_selected = (current_control_index == n);
            int shifted_index = 0;
            if (ImGui::Selectable(control_items[n], is_selected))
            {
                current_control_index = n;
                if (n == 0) {
                    cam_control_mode = (ControlMode)shifted_index;
                    p_graphics_manager->GetPActiveCam()->SetControllerMode((ControlMode)current_control_index);
                }
                else {
                    shifted_index = 1 << (current_control_index - 1);
                    cam_control_mode = (ControlMode)shifted_index;
                    p_graphics_manager->GetPActiveCam()->SetControllerMode((ControlMode)shifted_index);
                }
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            
        }
        ImGui::EndCombo();
    }

    const char* proj_items[] = { "PERSPECTIVE", "ORTHOGRAPHIC" };
    static int current_proj_index = 0;
    static ProjectionMode cam_proj_mode = ProjectionMode::PERSPECTIVE;
    const char* proj_label = proj_items[current_proj_index];

    // Draw Combobox to swtich perspective if in Viewer Mode
    if ((cam_control_mode == ControlMode::VIEWER) || (cam_control_mode == ControlMode::TOP_DOWN))
    {
        p_graphics_manager->GetPActiveCam()->SetProjectionMode((ProjectionMode)current_proj_index);
        if (ImGui::BeginCombo("Projection Mode", proj_label))
        {
            for (int n = 0; n < IM_ARRAYSIZE(proj_items); n++)
            {
                bool is_selected = (current_proj_index == n);
                if (ImGui::Selectable(proj_items[n], is_selected))
                {
                    current_proj_index = n;
                    cam_proj_mode = ((ProjectionMode)current_proj_index);
                    p_graphics_manager->GetPActiveCam()->SetProjectionMode((ProjectionMode)current_proj_index);
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();


            }
            ImGui::EndCombo();
        }

        
    }

    if (cam_proj_mode == ProjectionMode::PERSPECTIVE)
    {
        ImGui::DragFloat("Camera FOV", &active_cam->GetFOV(), 1.0f, 25.0f, 170.0f);
    }
    else if (cam_proj_mode == ProjectionMode::ORTHOGRAPHIC)
    {
        ImGui::DragFloat("Orthographic Size", &active_cam->GetOrthoSize(), 1.0f, 3.0f, 100.0f);
    }

    ImGui::End();
}
#ifdef _PROTOTYPE
/*
* Spawns the prototype menu bar.
* If a prototype is running, shows the currently running one
* Else, can Launch a specific prototype
*/
void SIK::ImGuiWindow::ShowPrototypeMenuBar() {
    if (p_game_manager->IsPrototypeRunning())
    {
        if (ImGui::BeginMenu("Prototype - Running"))
        {
            if (ImGui::MenuItem("End Running Prototype"))
                p_game_manager->EndActivePrototype();
            ImGui::EndMenu();
        }
    }
    else
    {
        if (ImGui::BeginMenu("Prototype"))
        {
            Uint8 prototype_indx = 0;
            for (auto& prototype_name : p_game_manager->getPrototypeNames()) {
                if (ImGui::MenuItem(prototype_name.c_str()))
                {
                    p_game_manager->LaunchPrototype(prototype_indx);
                }
                prototype_indx++;
            }
            ImGui::EndMenu();
        }
    }
}
#endif // _PROTOTYPE

void ImGuiWindow::ShowTimerTool() {
    ImGui::Begin("Timer Tool");
    ImGui::SliderInt("Set Total Time", &total_time, 0, 100);
    ImGui::End();
}

void ImGuiWindow::Render() {
    ShowMenu();

    if(show_audio_functions)    ShowAudioFunctions();
    if(show_performance)        ShowPerformance();
    if(show_graphics_functions) ShowGriphicsFunctions();
    if(show_camera_functions)   ShowCameraControls();   
    if(show_scene_editor)       ShowSceneEditor();
    if (show_timer_tool)        ShowTimerTool();

    if (p_input_manager->IsKeyReleased(SDL_SCANCODE_F1)) {
            show_scene_editor = !show_scene_editor;}

    if (p_input_manager->IsKeyReleased(SDL_SCANCODE_F3)) {
        show_camera_functions = !show_camera_functions;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(p_graphics_manager->GetPWindow(), p_graphics_manager->GetGLContext());
    }
}

void ImGuiWindow::Destroy() {            
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

float ImGuiWindow::GetTimerValue() {
    return static_cast<float>(total_time);
}
