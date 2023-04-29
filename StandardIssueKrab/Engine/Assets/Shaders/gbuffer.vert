#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 norm_inverse;
uniform mat4 view_inverse;

//Draw ID to check if we are drawing background
uniform int draw_id;

out vec4 world_pos;
out vec3 world_norm;
out vec2 tex_coords;

out vec3 eye_vec;

out vec3 tangent_vec;

void main()
{
    //Drawing background
    if (draw_id == 2) {
        gl_Position = vec4(2 * pos, 1.0);
        world_pos = gl_Position;
        tex_coords = uv;
        return;
    }

    world_pos = model * vec4(pos, 1.0);
    world_norm = (norm_inverse * vec4(norm, 1.0)).xyz;
    tex_coords = uv;
	gl_Position = proj * view * world_pos;
    
    world_pos.w = gl_Position.w;

    vec3 eye_pos = (view_inverse * vec4(0,0,0,1)).xyz;
    eye_vec =  eye_pos - world_pos.xyz;

    tangent_vec = tangent;
}