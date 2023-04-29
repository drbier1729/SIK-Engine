#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 world_pos;
    vec3 world_norm;
} gs_in[];

const float MAGNITUDE = 0.4;

uniform mat4 view;
uniform mat4 proj;

void main()
{   
    // Get points of triangle
    vec3 p0 = gs_in[0].world_pos;
    vec3 p1 = gs_in[1].world_pos;
    vec3 p2 = gs_in[2].world_pos;

    // Calulate center point
    vec3 center = (p0 + p1 + p2) / 3;
    gl_Position = proj * view * vec4(center, 1);
    EmitVertex();

    // Calculate triangle plane vectors
    vec3 v01 = p1 - p0;
    vec3 v02 = p2 - p0;

    // Calculate Normal
    vec3 norm = normalize(cross(v01, v02));
    gl_Position = proj * view * vec4(center + (norm * MAGNITUDE), 1);
    EmitVertex();
    EndPrimitive();
    
}