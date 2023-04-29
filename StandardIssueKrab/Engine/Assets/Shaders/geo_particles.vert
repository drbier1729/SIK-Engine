#version 330 core

layout (location = 0) in vec3 inVert;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inPosAndSize;
layout (location = 3) in vec4 inColor;

out vec4 Color;
out vec2 TexCoords;

out VS_OUT {
    vec3 gl_Position;
    vec4 inPosAndSize;
} vs_out;

void main()
{
    vs_out.inPosAndSize = inPosAndSize;
    vs_out.gl_Position = inVert;
    gl_Position = vec4(inVert, 0.0f);
    
    Color = inColor;
    TexCoords = inUV;
} 