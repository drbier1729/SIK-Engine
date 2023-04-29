#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 uv;

out vec2 tex_coords;

void main()
{
    // vertex of plane [-0.5, 0.5]
    gl_Position = vec4(2 * pos, 1.0);
    tex_coords = uv;
}