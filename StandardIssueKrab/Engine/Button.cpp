#include "stdafx.h"
#include "InputAction.h"
#include "Button.h"
#include "MemoryResources.h"
#include "GUIObjectManager.h"
#include "InputManager.h"
#include "Behaviour.h"

#include "MemoryResources.h"
#include "GUIObjectManager.h"
#include "InputManager.h"
#include "AudioManager.h"

void DefaultAction() {
    SIK_INFO("Default button action");
}

//Ctor for button
Button::Button(const Vec2& _global_space_coords, const Vec2& _dimensions) :
    GUIObject(_global_space_coords, _dimensions), action(nullptr),
    is_clickable(true), is_clicked(false), is_clicked_and_released(false) {
    SetScriptState();
}

//Dtor for button
Button::~Button() {
}

void Button::Update(Float32 dt) {
    GUIObject::Update(dt);
    if (not IsActive())
        return;

    is_clicked_and_released = false;
    if (is_clickable) {
        //If the button is not clickbable all functions are essentially disabled. Just render.
        if (!GetHighlightState()) {
            /*
            * If the button is not currently highlighted make sure
            * it's not in the clicked state
            */
            is_clicked = false;
            SetAlternateRender(false);
        }
        else {
            /*
            * Button is highlighted.
            * Check for action released and pressed.
            */
            if (!is_clicked) {
                //Button is not currently clicked down. Check for action pressed.
                if (p_gui_object_manager->gui_action_map.IsActionPressed(InputAction::Actions::ACTION_1) || 
                    (p_input_manager->IsMouseButtonPressed(SDL_MOUSE_BUTTON_LEFT) && CheckPosOverlap(p_input_manager->GetMousePos()))) {
                    is_clicked = true;
                    SetAlternateRender(true);
                }
            }
            else {
                //Button is currently clicked. If button is released, perform action.
                if (p_gui_object_manager->gui_action_map.IsActionReleased(InputAction::Actions::ACTION_1) ||
                    (p_input_manager->IsMouseButtonReleased(SDL_MOUSE_BUTTON_LEFT) && CheckPosOverlap(p_input_manager->GetMousePos()))) {
                    is_clicked = false;
                    is_clicked_and_released = true;
                    SetAlternateRender(false);

                    action();
                }
            }
        }
    }
    SetScriptState();
}

/*
* Returns the clickable state of the button
* Return: bool - True if clickable
*/
Bool Button::GetClickableState() const {
    return is_clickable;
}

//Sets the clickable state to false
void Button::DisableClickable() {
    is_clickable = false;
}

//Sets the clickable state to true
void Button::EnableClickable() {
}

/*
* Sets a callback as the action to perform when the button is clicked
* Returns: void
*/
void Button::SetAction(ButtonAction _action) {
    action = _action;
}

void Button::SetDefaultAction() {
    action = &DefaultAction;
}

void Button::SetScriptState() {
    if (p_behaviour != nullptr) {
        for (auto& script : p_behaviour->scripts) {
            script.script_state.set("is_clicked", is_clicked);
            script.script_state.set("is_clicked_and_released", is_clicked_and_released);
            script.script_state.set("is_clickable", is_clickable);
        }
    }
}

void Button::PlayClickSound() {
    p_audio_manager->PlayAudio("BUTTON_CLICK"_sid,
        p_audio_manager->sfx_chanel_group,
        70.0f,
        0.5f,
        false,
        0);
}

void Button::PlayDisabledClickSound() {
    p_audio_manager->PlayAudio("BUTTON_DISABLED_CLICK"_sid,
        p_audio_manager->sfx_chanel_group,
        70.0f,
        0.5f,
        false,
        0);
}


