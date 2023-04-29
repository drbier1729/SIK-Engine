#include "stdafx.h"
#include "Texture.h"
#include "stb_image.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in Texture.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }

////////////////////////////////////////////////////////////////////////////////
// Texture methods
////////////////////////////////////////////////////////////////////////////////

Texture::Texture(Int32 w, Int32 h, Int32 chs, unsigned char* data)
	: width{ w }, height{ h }, channel_count{ chs },
	texture_id{ GenTexture(GL_LINEAR, GL_LINEAR, w, h, chs, data) }, texture_path{}, texture_type{}
{ }

Texture::Texture(Texture&& src) noexcept
	: width{ src.width }, height{ src.height }, channel_count{ src.channel_count },
	texture_id{ src.texture_id }, texture_path{}, texture_type{}
{
	src.width = 0;
	src.height = 0;
	src.channel_count = 0;
	src.texture_id = 0;
}

Texture& Texture::operator=(Texture&& rhs) noexcept {
	if (this == &rhs) { return *this; }

	glDeleteTextures(1, &texture_id);

	width = rhs.width;
	height = rhs.height;
	channel_count = rhs.channel_count;
	texture_id = rhs.texture_id;
	
	texture_path = rhs.texture_path;
	texture_type = rhs.texture_type;

	rhs.width = 0;
	rhs.height = 0;
	rhs.channel_count = 0;
	rhs.texture_id = 0;

	rhs.texture_path = "";
	rhs.texture_type = "";

	return *this;
}

Texture::~Texture() {
	glDeleteTextures(1, &texture_id);
}

/*
* Makes a texture accessible to a program specified by the program_id
* The name of the sampler2D variable in the shader program is specified by name
* Return: void
*/
void Texture::Bind(const Uint32 unit, const Uint32 program_id, const char* name) const {
	Uint32 tex = (texture_id == 0) ? default_id : texture_id;

	glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
	glBindTexture(GL_TEXTURE_2D, tex);

	int loc = glGetUniformLocation(program_id, name);
	glUniform1i(loc, unit);

	CHECKERROR;
}

//Unbind a texture after use
void Texture::Unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
* Cals glGenerateMipmap to create the default mipmap for the texture
* Useful for dynamic LOD
* Return: void
*/
void Texture::GenerateDefaultMipmap() const {
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECKERROR;
}

////////////////////////////////////////////////////////////////////////////////
// Texture static methods
////////////////////////////////////////////////////////////////////////////////

Uint32 Texture::default_id = 0;

void Texture::InitDefault() {
	static constexpr const unsigned char default_tex[2][2][4] = {
	{{255, 0, 0, 255}, {0, 0, 255, 255}},
	{{0, 0, 255, 255}, {255, 0, 0, 255}}
	};

	default_id = GenTexture(GL_NEAREST, GL_NEAREST, 2, 2, 4, (unsigned char*)(default_tex));
}

void Texture::FreeDefault() {
	glDeleteTextures(1, &default_id);
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

Uint32 Texture::GenTexture(GLuint min_filter, GLuint mag_filter,
	Int32 width, Int32 height, Int32 channel_count, unsigned char* data) {

	if (data == nullptr) { return default_id; }

	Uint32 texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	GLuint mode = GL_SRGB;
	GLuint format = GL_RGB;

	if (channel_count == 4) {
		mode = GL_SRGB_ALPHA;
		format = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECKERROR;

	return texture_id;
}
