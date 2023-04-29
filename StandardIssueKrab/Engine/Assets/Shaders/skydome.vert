#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 iView;

out vec3 eye_vec;

void main()
{
    vec3 world_pos = (model * vec4(pos, 1.0)).xyz;
    gl_Position = proj * view * model * vec4(pos, 1.0);
    
    
    vec3 eye_pos = (iView*vec4(0,0,0,1)).xyz;
    eye_vec = eye_pos - world_pos;
}