#version 330 core

uniform sampler2D texture_sampler;

in vec3 world_pos;
in vec3 eye_vec;

out vec4 FragColor;

#define PI 3.1415926538

void main()
{
  vec3 V = normalize(eye_vec);
  vec2 tex_coords = vec2((- 1.0) * atan(V.z,V.x)/(2*PI), acos(-V.y)/PI); 	

  FragColor.xyz = texture2D(texture_sampler, tex_coords).xyz;    
}