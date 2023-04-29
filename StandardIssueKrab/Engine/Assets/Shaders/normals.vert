#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

uniform mat4 model;
uniform mat4 norm_inverse;

out VS_OUT {
    vec3 world_pos;
    vec3 world_norm;
} vs_out;

void main()
{
    vs_out.world_pos = (model * vec4(pos, 1.0)).xyz;
    vs_out.world_norm = (norm_inverse * vec4(norm, 1.0)).xyz;
	gl_Position = model * vec4(pos, 1.0);
}