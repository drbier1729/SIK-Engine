#include "stdafx.h"
#include "Engine/InputAction.h"
#include "Engine/InputManager.h"
#include "Engine/MemoryResources.h"
#include "Engine/GraphicsManager.h"
#include "CountdownTimer.h"
#include "Engine/PrototypeInterface.h"
#include "Engine/GUIObject.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GUIText.h"
#include "Engine/AudioManager.h"
#include "Engine/ImGuiWindow.h"
#include "Player.h"

extern Player* player;

CountdownTimer::CountdownTimer()
{
    game_time = p_imgui_window->GetTimerValue();
}

CountdownTimer::~CountdownTimer()
{
}

void CountdownTimer::Update(float dt)
{
    if (test_actions.IsActionPressed(InputAction::Actions::ACTION_1))
    {
        timer_is_start = true;
        p_audio_manager->PlayAudio("DRIVEBACKGROUND"_sid, p_audio_manager->background_chanel);
        p_audio_manager->SetVolume(0.3f, p_audio_manager->background_chanel);
    }

    if (timer_is_start)
        TimerUpdate(dt);

}

void CountdownTimer::TimerUpdate(float dt)
{
    game_time -= 1.0f * dt;

    std::string string_text = std::to_string(game_time);
    string_text.resize(4);
    const char* display_text = string_text.c_str();

    p_gui_object_manager->GetTextObject(1)->SetText(display_text);

    if (game_time < 0.0f)
    {
        timer_is_start = false;
        game_time = p_imgui_window->GetTimerValue();
        p_audio_manager->background_chanel->stop();
        ResetTimer();
        player->ResetPlayer();
    }
}

void CountdownTimer::ResetTimer()
{
    timer_is_start = false;
    game_time = p_imgui_window->GetTimerValue();

    std::string string_text = std::to_string(game_time);
    string_text.resize(4);
    const char* display_text = string_text.c_str();

    p_gui_object_manager->GetTextObject(1)->SetText(display_text);

    p_audio_manager->background_chanel->stop();
}

bool CountdownTimer::GetTimeState()
{
    return timer_is_start;
}
