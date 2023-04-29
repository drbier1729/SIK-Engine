#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 norm_inverse;
uniform mat4 shadow_mat;

out vec3 world_pos;
out vec3 world_norm;
out vec2 tex_coords;
out vec4 shadow_pos;

void main()
{
    world_pos = (model * vec4(pos, 1.0)).xyz;
    world_norm = (norm_inverse * vec4(norm, 1.0)).xyz;
    tex_coords = uv;
	gl_Position = proj * view * model * vec4(pos, 1.0);
    shadow_pos = shadow_mat * model * vec4(pos, 1.0);
}