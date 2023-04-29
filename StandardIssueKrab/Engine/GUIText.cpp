#include "stdafx.h"
#include "FontTextures.h"
#include "GUIText.h"

GUIText::GUIText(const Vec2& _global_space_coords, const Vec2& _dimensions) : 
	GUIObject(_global_space_coords, _dimensions), p_font_textures(nullptr) {
}

void GUIText::Update(Float32 dt) {
	GUIObject::Update(dt);
    SetAlternateRender( GetHighlightState() );
}

void GUIText::SetFontTextures(FontTextures* _p_font_textures) {
	p_font_textures = _p_font_textures;
}

FontTextures* GUIText::GetFontTextures() const {
	return p_font_textures;
}

void GUIText::SetText(const char* txt) {
	text = txt;
}

const String& GUIText::GetText() const {
	return text;
}
