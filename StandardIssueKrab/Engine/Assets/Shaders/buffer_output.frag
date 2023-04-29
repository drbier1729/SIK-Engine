#version 330 core

out vec4 FragColor;

uniform sampler2D fbo_texture;

uniform uint buffer_width, buffer_height;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(buffer_width, buffer_height);
	FragColor = texture(fbo_texture, uv);
}