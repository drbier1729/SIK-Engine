#pragma once
#include "GameObject.h"

namespace SIK {
class PerformanceManager;

class ImGuiWindow
{
private:	
#ifdef _PROTOTYPE
    Vector<String> TestNames;
    Vector<String> PrototypeNames;
#endif // _PROTOTYPE
    Vector<float> fps;
    int total_time;
    char* current_filename;

private:
    void ShowSceneEditor();
    void ShowMenu();
    void ShowPerformance();
    void ShowAudioFunctions();
    void ShowGriphicsFunctions();
    void ShowCameraControls();
#ifdef _PROTOTYPE
    void ShowPrototypeMenuBar();
#endif // _PROTOTYPE
    void ShowTimerTool();

    inline static Vector<String> ScriptNames;
    inline static Vector<String> CompNames;
    inline static Vector<String> ArchNames;
    static void ShowObjectComponent(GameObject* GameObject);
public:
    ImGuiWindow();
    ~ImGuiWindow();
    
    void Init();
    void HandleSDLEvent(SDL_Event& e);
    void BeginRender();
    void Render();
    void Destroy();

    float GetTimerValue();
};


}

extern SIK::ImGuiWindow* p_imgui_window;