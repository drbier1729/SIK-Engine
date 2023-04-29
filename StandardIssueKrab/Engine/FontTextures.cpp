#include "stdafx.h"
#include "FontTextures.h"

#include <ft2build.h>
#include FT_FREETYPE_H

/*Creates a map of characters and their associated textures
* that can be used to render a character using the font type
* specified in the argument
*/
FontTextures::FontTextures(const char* ttf_file_path, Uint8 _font_pixel_height,
						   PolymorphicAllocator res_manager_alloc) :
	font_pixel_height(_font_pixel_height), character_map(res_manager_alloc) {
    FT_Library ft = nullptr;
    FT_Face face = nullptr;

    FT_Error err = FT_Init_FreeType(&ft);
    SIK_ASSERT(err == 0, "Failed to Init FreeType");

    err = FT_New_Face(ft, ttf_file_path, 0, &face);
    SIK_ASSERT(err == 0, "Failed to create FreeType face");

    FT_Set_Pixel_Sizes(face, 0, _font_pixel_height);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Create a texture for the first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++)
    {
        //Load glyph onto face
        err = FT_Load_Char(face, c, FT_LOAD_RENDER);
        SIK_ASSERT(err == 0, "Failed to load freetype character into face");

        //Generate texture for character and ship to GPU
        GLuint tex_id;
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED, //Single channel
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Save the tex_id and tex information in the map
        FontCharacter character = {
            tex_id,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        character_map.emplace(c, character);
    }
    //Restore default UNPACK_AlIGNMENT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Int8 FontTextures::GetPixelHeight() {
    return font_pixel_height;
}

void FontCharacter::Bind(GLuint unit, GLuint shader_program, const char* name) {
    glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
    glBindTexture(GL_TEXTURE_2D, texture_id);
    int loc = glGetUniformLocation(shader_program, name);
    glUniform1i(loc, unit);
}
