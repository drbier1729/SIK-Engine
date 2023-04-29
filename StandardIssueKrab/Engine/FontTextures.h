#pragma once

//struct that represents a single character texture. 
struct FontCharacter {
	GLuint texture_id;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
	void Bind(GLuint unit, GLuint shader_program, const char* name);
};

class FontTextures {
public:
	Int8 font_pixel_height;
	UnorderedMap<char, FontCharacter> character_map;
	/*Creates a map of characters and their associated textures
	* that can be used to render a character using the font type
	* specified in the argument
	*/
	FontTextures(const char* ttf_file_path, Uint8 _font_pixel_height,
				 PolymorphicAllocator res_manager_alloc);

	/*
	* Gets the height of the font textures created in pixels
	* Returns: Int8 - the height
	*/
	Int8 GetPixelHeight();
};

