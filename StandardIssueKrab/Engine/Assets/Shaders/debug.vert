#version 330 core

in vec4 vertex;

uniform mat4 ModelTr, Proj, View;

void main()
{
    gl_Position = Proj * View * ModelTr * vertex;
}