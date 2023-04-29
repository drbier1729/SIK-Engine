//Vertex shader for UI elements

#version 330 core

layout(location=0) in vec2 in_position;
layout(location=1) in vec2 in_TexCoords;

uniform mat4 ortho_projection;
uniform mat4 transform_mat;

out vec2 ex_TextCoord;

void main() {
	gl_Position = ortho_projection * transform_mat * vec4(in_position, 0.0, 1.0);
	ex_TextCoord = in_TexCoords;
}
