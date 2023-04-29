#pragma once

#include "GUIObject.h"

class Texture;

class GUIRenderer {
private:
	GUIObject* p_owner_obj;
	bool enabled;
	bool is_text;
	GLuint quad_vao, point_buffer, tex_buffer, index_buffer;
	Vector<Vector<GLfloat>> vertex_coords;
	void GenerateQuadVao(const Ivec2& _dimensions);
	void GenerateDynamicQuadVao();
	void GenerateTextVertices();
	friend class GUIObject;
public:
	//Bool to check if we need to render this as highlighted
	bool is_highlighted;

	//Bool to check if we need to render the alternate texture
	bool is_alternate;

	GUIRenderer(GUIObject* _p_owner_obj);
	~GUIRenderer();
	
	//Enable rendering of this gui
	void Enable();

	//Disable rendering of this gui
	void Disable();

	void Draw();
	//Call to specifically draw Text
	void DrawTextFont();
};

