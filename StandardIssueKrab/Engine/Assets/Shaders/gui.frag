//Fragment shader code for UI elements

#version 330 core

in vec2 ex_TextCoord;

uniform int tex_enabled;
uniform int alt_render;
uniform int highlighted;
uniform int is_text;
uniform int is_transition;

uniform float cutoff;
uniform float ease;

uniform vec4 color;

uniform sampler2D texture_sampler;
uniform sampler2D transition_sampler;

layout(location = 0)out vec4 out_Color;

void main() {
	//Mode specified if we are filling with colors or textures.
	out_Color = color;

	if (tex_enabled == 1) {
		if (is_text == 1) {
			out_Color = vec4(color.rgb, texture(texture_sampler, ex_TextCoord).r);
			if (out_Color.a < 0.01)
				discard;
			//No Highlights needed for text
			return;
		}
		else {
			float gamma = 1.8f;
			out_Color = texture(texture_sampler, ex_TextCoord);
			out_Color.rgb = pow(out_Color.rgb, vec3(1.0f/gamma));
		}
	}

	//If highlighted is enabled then make is slightly brighter
	if (highlighted == 1 && alt_render != 1) {
		out_Color.rgb = out_Color.rgb*1.5;
		if (ex_TextCoord.x < 0.05 || 
			ex_TextCoord.y < 0.05 || 
			ex_TextCoord.x > 0.95 || 
			ex_TextCoord.y > 0.95 )
			out_Color.rgb = out_Color.rgb*1.5;
	}

	if (is_transition == 1) {
		vec2 center = vec2(0.5);
		float map = texture(transition_sampler, ex_TextCoord).r;
		float c_cutoff = cutoff + cutoff / ease;
		out_Color = mix(out_Color,
			vec4(0.0f), 
			clamp((c_cutoff - map) * ease, 0.0, 1.0));
	}
}