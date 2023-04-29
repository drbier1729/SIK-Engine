#include "stdafx.h"
#include "FadeState.h"

#include "Engine/GUIObjectManager.h"
#include "Engine/FadePanel.h"
#include "Engine/GameStateManager.h"
#include "Engine/AudioManager.h"

FadeState::FadeState(const char* fade_json, Float32 _fade_duration, 
					 Bool _fade_out, Bool _cycle_fade) :
	fade_duration(_fade_duration), fade_completed(_fade_duration), 
	fade_out(_fade_out), cycle_fade(_cycle_fade), fade_effect_id(0) {
	auto vec = p_gui_object_manager->CreateGUIFromFile(fade_json);
	p_fade = static_cast<FadePanel*>(vec.back());
	p_fade->Disable();
}

void FadeState::Enter() {
	fade_completed = fade_duration;
	p_fade->Enable();
	
	if (fade_out) 
		p_fade->SetCutoff(0.0f);
	else
		p_fade->SetCutoff(1.0f);

	fade_effect_id = p_audio_manager->PlayAudio("TRANSITION_SFX"_sid,
			p_audio_manager->sfx_chanel_group,
			30.0f,
			1.0f,
			false,
			0);
}

void FadeState::Exit() {
	p_fade->Disable();
	p_audio_manager->Stop(fade_effect_id);
}

void FadeState::Update(Float32 dt) {
	fade_completed -= dt;

	if (fade_completed <= 0.0f) {
		if (fade_out) {
			Exit();
		}
		return;
	}

	float fade_ratio = (fade_completed / fade_duration);

	if (cycle_fade) {
		fade_ratio *= 2;
		if (fade_ratio > 1.0f)
			fade_ratio = 2.0f - fade_ratio;
	}

	if (fade_out)
		p_fade->SetCutoff(1.0f - fade_ratio);
	else
		p_fade->SetCutoff(fade_ratio);
}

Bool FadeState::IsComplete() const {
	return fade_completed <= 0.0f;
}

void FadeState::Reset() {
	fade_completed = fade_duration;
}

void FadeState::Reset(Float32 new_fade_duration)
{
	fade_duration = new_fade_duration;
	fade_completed = fade_duration;
}
