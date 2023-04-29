#version 330 core

layout (location = 0) in vec3 inVert;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inPosAndSize;
layout (location = 3) in vec4 inColor;

out vec4 Color;
out vec2 TexCoords;

uniform mat4 ProjView;
uniform vec3 CamRight, CamUp;

void main()
{
    float sz = inPosAndSize.w;
    vec3 pos = inPosAndSize.xyz;
    vec4 worldPos = vec4(pos + CamRight * inVert.x * sz + CamUp * inVert.y * sz, 1.0);
    
    gl_Position = ProjView * worldPos;
    Color = inColor;
    TexCoords = inUV;
} 