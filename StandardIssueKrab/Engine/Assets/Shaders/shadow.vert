#version 330 core

layout (location = 0) in vec3 pos;

out vec4 shadow_pos;

uniform mat4 shadow_proj, shadow_view, model;

void main()
{      
    shadow_pos = shadow_proj * shadow_view * model * vec4(pos, 1.0);
    gl_Position = shadow_pos;
}