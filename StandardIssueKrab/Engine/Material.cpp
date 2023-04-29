#include "stdafx.h"
#include "Material.h"
#include "Texture.h"
#include "GraphicsManager.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in Material.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }

Material* Material::bound_material = nullptr;

void Material::Use() {
	
	if (Material::bound_material == this)
		return;

	// TODO: Read Uniforms from file
	// Store uniforms in a map
	// Add proper uniform setting methods
	GLuint bound_shader = p_graphics_manager->GetBoundShader();

	p_graphics_manager->SetUniform(bound_shader, diffuse, "diffuse_color");
	p_graphics_manager->SetUniform(bound_shader, specular, "specular_color");
	p_graphics_manager->SetUniform(bound_shader, glossiness, "glossiness");
	p_graphics_manager->SetUniform(bound_shader, emission, "emission_color");
	
	if (base_color != nullptr)
	{
		p_graphics_manager->SetUniform(bound_shader, 1, "has_color_tex");
		base_color->Bind(0, bound_shader, "base_color_tex");
	}
	else {
		p_graphics_manager->SetUniform(bound_shader, 0, "has_color_tex");
	}

	
	if (normal_map != nullptr)
	{
		p_graphics_manager->SetUniform(bound_shader, 1, "has_normal_map");
		normal_map->Bind(1, bound_shader, "normal_map");
	}
	else {
		p_graphics_manager->SetUniform(bound_shader, 0, "has_normal_map");
	}

	Material::bound_material = this;
}

void Material::Unuse() {
	GLuint bound_shader = p_graphics_manager->GetBoundShader();
	if (base_color != nullptr)
	{
		p_graphics_manager->SetUniform(bound_shader, 0, "has_color_tex");
		base_color->Unbind();
	}

	if (normal_map != nullptr)
	{
		p_graphics_manager->SetUniform(bound_shader, 0, "has_normal_map");
		normal_map->Unbind();
	}

	Material::bound_material = nullptr;
}