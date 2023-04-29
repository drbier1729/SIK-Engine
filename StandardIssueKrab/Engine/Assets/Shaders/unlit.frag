#version 330 core

in vec3 world_pos;
in vec3 world_norm;
in vec2 tex_coords;

out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);	
}