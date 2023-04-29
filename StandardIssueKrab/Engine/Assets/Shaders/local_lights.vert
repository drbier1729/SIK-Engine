#version 330 core

uniform mat4 model, view, proj;

layout (location = 0) in vec3 pos;

out vec3 out_pos;

void main()
{
    gl_Position = proj * view * model * vec4(pos, 1.0f);
    out_pos = gl_Position.xyz;
}
