#pragma once
#include "GUIObject.h"
class GUIText : public GUIObject {
private:
	class FontTextures* p_font_textures;
	String text;
public:
	GUIText(const Vec2& _global_space_coords, const Vec2& _dimensions = Vec2(0, 0));
	~GUIText() = default;

	void Update(Float32 dt) override;

	//Sets the FontTextures pointer
	void SetFontTextures(FontTextures* _p_font_textures);
	//Get the FontTextures pointer
	FontTextures* GetFontTextures() const;
	//Set Text to display
	void SetText(const char* txt);
	//Get the text to display
	const String& GetText() const;
};

