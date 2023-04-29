#version 330 core

in vec2 tex_coords;

uniform sampler2D texture_sampler;
uniform bool showTexture;
uniform vec3 color1;
uniform vec3 color2;

out vec4 FragColor;

void main()
{
  if(showTexture) {
    //                                                        fix upside-down
    FragColor = texture2D(texture_sampler, vec2(tex_coords.x, 1.0 - tex_coords.y));
  }
  else {
    float t = tex_coords.y;
    FragColor = vec4(color1 * (1 - t) + color2 * t, 1.0);
  }   
}