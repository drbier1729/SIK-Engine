#include "stdafx.h"
#include "Texture.h"
#include "MemoryResources.h"
#include "GraphicsManager.h"
#include "GUIText.h"
#include "FontTextures.h"
#include "GUIRenderer.h"
#include "FadePanel.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in GUIRenderer.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }

void GUIRenderer::GenerateQuadVao(const Ivec2& _dimensions) {
	//Create a VAO
	glGenVertexArrays(1, &quad_vao);
	//Use the same VAO for all the following operations
	glBindVertexArray(quad_vao);

	//Create a vertex buffer
	Vector<float> vertices;

	vertices.push_back(static_cast<float>(0));
	vertices.push_back(static_cast<float>(0));
	//Bottom left vertex
	vertices.push_back(static_cast<float>(0));
	vertices.push_back(static_cast<float>(_dimensions.y));
	//Bottom right vertex
	vertices.push_back(static_cast<float>(_dimensions.x));
	vertices.push_back(static_cast<float>(_dimensions.y));
	//Top right vertex
	vertices.push_back(static_cast<float>(_dimensions.x));
	vertices.push_back(static_cast<float>(0));

	//Create a texture buffer
	Vector<float> texture_coords;
	//Top left vertex
	texture_coords.push_back(0.0f);
	texture_coords.push_back(0.0f);
	//Bottom left vertex
	texture_coords.push_back(0.0f);
	texture_coords.push_back(1.0f);
	//Bottom right vertex
	texture_coords.push_back(1.0f);
	texture_coords.push_back(1.0f);
	//Top right vertex
	texture_coords.push_back(1.0f);
	texture_coords.push_back(0.0f);

	//Create a buffer for all the vertex points
	glGenBuffers(1, &point_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;

	//Create another buffer for all the texture coords
	glGenBuffers(1, &tex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * texture_coords.size(), texture_coords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;

	//Create index buffer
	std::vector<GLuint> index_data = { 0, 1, 2, 0, 2, 3 };
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
		sizeof(GLuint) * index_data.size(), index_data.data(), GL_STATIC_DRAW);
	CHECKERROR;
	glBindVertexArray(0);
}

void GUIRenderer::GenerateDynamicQuadVao() {
	int vertices_count = 4;
	int positions_count = 3;
	int tex_coords_count = 2;

	//Create a VAO
	glGenVertexArrays(1, &quad_vao);
	//Use the same VAO for all the following operations
	glBindVertexArray(quad_vao);

	//Create a continguous buffer for all the vertices/points
	glGenBuffers(1, &point_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * vertices_count * positions_count,
		NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;

	//Create a texture buffer
	Vector<float> texture_coords;
	//Top left vertex
	texture_coords.push_back(0.0f);
	texture_coords.push_back(0.0f);
	//Bottom left vertex
	texture_coords.push_back(0.0f);
	texture_coords.push_back(1.0f);
	//Bottom right vertex
	texture_coords.push_back(1.0f);
	texture_coords.push_back(1.0f);
	//Top right vertex
	texture_coords.push_back(1.0f);
	texture_coords.push_back(0.0f);

	//Create another buffer for all the texture coords
	glGenBuffers(1, &tex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * texture_coords.size(), texture_coords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;

	//IBO data
	Vector<GLuint> indexData;

	indexData.push_back(0);
	indexData.push_back(1);
	indexData.push_back(2);
	indexData.push_back(0);
	indexData.push_back(2);
	indexData.push_back(3);

	//Create IBO
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint),
		&indexData[0], GL_STATIC_DRAW);
	CHECKERROR;
	glBindVertexArray(0);

}

void GUIRenderer::GenerateTextVertices() {
	GUIText* text_obj = dynamic_cast<GUIText*>(p_owner_obj);

	String::const_iterator c;
	float char_x_pos = static_cast<float>(0);
	float char_y_pos = static_cast<float>(0);
	for (c = text_obj->GetText().begin(); c != text_obj->GetText().end(); c++)
	{
		FontCharacter ch = text_obj->GetFontTextures()->character_map[*c];

		float xpos = char_x_pos + ch.bearing.x;
		float ypos = char_y_pos - (ch.size.y - ch.bearing.y);

		float w = static_cast<float>(ch.size.x);
		float h = static_cast<float>(ch.size.y);
		Vector<GLfloat> temp_vec;
		temp_vec.push_back(xpos);
		temp_vec.push_back(ypos - h);
		temp_vec.push_back(0.0f);

		temp_vec.push_back(xpos);
		temp_vec.push_back(ypos);
		temp_vec.push_back(0.0f);

		temp_vec.push_back(xpos + w);
		temp_vec.push_back(ypos);
		temp_vec.push_back(0.0f);

		temp_vec.push_back(xpos + w);
		temp_vec.push_back(ypos - h);
		temp_vec.push_back(0.0f);

		vertex_coords.push_back(temp_vec);

		//(advance is number of 1/64 pixels)
		char_x_pos += (ch.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)
	}
}

GUIRenderer::GUIRenderer(GUIObject* _p_owner_obj) :
	p_owner_obj(_p_owner_obj),
	enabled(true), is_highlighted(false), is_alternate(false), is_text(false) {
	if (dynamic_cast<GUIText*>(p_owner_obj)) {
		is_text = true;
		GenerateDynamicQuadVao();
		GenerateTextVertices();
	}
	else {
		GenerateQuadVao(p_owner_obj->GetDimensions());
	}
}

GUIRenderer::~GUIRenderer() {
	glBindVertexArray(quad_vao);
	glDeleteBuffers(1, &point_buffer);
	glDeleteBuffers(1, &tex_buffer);
	glDeleteBuffers(1, &index_buffer);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &quad_vao);
	CHECKERROR;
}

//Enable rendering of this gui
void GUIRenderer::Enable() {
	enabled = true;
}

//Disable rendering of this gui
void GUIRenderer::Disable() {
	enabled = false;
}

void GUIRenderer::Draw() {
	//If disabled then do nothing
	if (!enabled || !p_owner_obj->IsActive())
		return;

	GLuint bound_shader = p_graphics_manager->GetBoundShader();
	Texture* p_texture = p_owner_obj->GetTexture();
	Texture* p_alt_texture = p_owner_obj->GetAltTexture();

	is_alternate = (is_highlighted && (p_alt_texture != nullptr));

	if (p_texture != nullptr) {
		p_graphics_manager->SetUniform(bound_shader, 1, "tex_enabled");
		if (is_alternate) {
			p_alt_texture->Bind(0, bound_shader, "texture_sampler");
		}
		else {
			p_texture->Bind(0, bound_shader, "texture_sampler");
		}		
	}
	else {
		p_graphics_manager->SetUniform(bound_shader, 0, "tex_enabled");
	}

	p_graphics_manager->SetUniform(bound_shader, is_alternate, "alt_render");
	p_graphics_manager->SetUniform(bound_shader, is_highlighted, "highlighted");
	p_graphics_manager->SetUniform(bound_shader, 0, "is_text");
	p_graphics_manager->SetUniform(bound_shader, p_owner_obj->GetTransform(), "transform_mat");

	FadePanel* fp = dynamic_cast<FadePanel*>(p_owner_obj);
	if (fp != nullptr) {
		p_graphics_manager->SetUniform(bound_shader, 1, "is_transition");
		p_graphics_manager->SetUniform(bound_shader, fp->GetCutoff(), "cutoff");
		p_graphics_manager->SetUniform(bound_shader, fp->GetEase(), "ease");
		p_alt_texture->Bind(1, bound_shader, "transition_sampler");
	}
	else {
		p_graphics_manager->SetUniform(bound_shader, 0, "is_transition");
	}
	

	//Set the color uniform
	p_graphics_manager->SetUniform(bound_shader, p_owner_obj->GetColor(), "color");
	if (is_text) {
		DrawTextFont();
	}
	else {
		glBindVertexArray(quad_vao);

		//6 indeces per quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	}
	CHECKERROR;
	glBindVertexArray(0);
}

void GUIRenderer::DrawTextFont() {
	if (!enabled || !p_owner_obj->IsActive())
		return;

	GUIText* text_obj = dynamic_cast<GUIText*>(p_owner_obj);

	GLuint bound_shader = p_graphics_manager->GetBoundShader();
	p_graphics_manager->SetUniform(bound_shader, 1, "tex_enabled");
	p_graphics_manager->SetUniform(bound_shader, 1, "is_text");
	//Draw a separate quad for each text character
	String::const_iterator c;
	int indx = 0;
	
	for (c = text_obj->GetText().begin(); c != text_obj->GetText().end(); c++) {

		p_graphics_manager->SetDynamicBufferData(quad_vao, point_buffer, &vertex_coords[indx][0],
			sizeof(float) * vertex_coords[indx].size());

		text_obj->GetFontTextures()->character_map[*c].Bind(0, bound_shader, "texture_sampler");

		glBindVertexArray(quad_vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		indx++;
	}
}
