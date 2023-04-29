#version 330 core

in vec4 Color;
in vec2 TexCoords;

out vec4 FragColor;
  
uniform sampler2D Texture;
uniform int TextureEnabled;

void main()
{
    if (TextureEnabled == 1) {
        FragColor = Color * texture(Texture, TexCoords);
    }
    else { 
        FragColor = Color;
    }
}