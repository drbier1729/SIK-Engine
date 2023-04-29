#include "stdafx.h"
#include "FadePanel.h"
#include "GraphicsManager.h"
#include "GUIObjectManager.h"

FadePanel::FadePanel(const Ivec2& _global_space_coords, const Ivec2& _dimensions) :
	GUIObject(Ivec2(0), 
		Ivec2(p_graphics_manager->GetWindowWidth(), p_graphics_manager->GetWindowHeight())),
	cutoff(0.0f), ease(6.0f) {
}

void FadePanel::Update(Float32 dt) {
	GUIObject::Update(dt);
	if (not IsActive())
		return;
}

Float32 FadePanel::GetCutoff() const {
	return cutoff;
}

Float32 FadePanel::GetEase() const {
	return ease;
}

void FadePanel::SetCutoff(Float32 _cutoff) {
	cutoff = _cutoff;
}

void FadePanel::SetEase(Float32 _ease) {
	ease = _ease;
}
