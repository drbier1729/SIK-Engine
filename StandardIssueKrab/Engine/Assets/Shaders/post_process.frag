/*
* Post processing fragment shader.
* Reads and additively blends the bloom buffer into the render buffer
* Performs tone mapping
* Performs gamma correction
*/

#version 430

uniform sampler2D render_buffer;
uniform sampler2D downsample_buffer;
uniform sampler2D upsample_buffer;

uniform int draw_buffer;

uniform uint width, height;

uniform float exposure;
uniform float gamma;

uniform int bloom_enabled;
uniform float bloom_factor;
uniform float bloom_mip_level;

layout(location = 0) out vec4 out_color;

void main() {
	vec2 uv = gl_FragCoord.xy / vec2(width, height);
	vec4 frag_color = texture(render_buffer, uv);
	vec4 bloom_color = texture(upsample_buffer, uv);

	if (draw_buffer == 0) { //Draw the fully post processed output
		if (bloom_enabled == 1)
			frag_color = frag_color + (bloom_color * bloom_factor);
		
		//Perform tone mapping
		frag_color.rgb = vec3(1.0) - exp(-frag_color.rgb*exposure);

		//Convert color back into sRGB space
		frag_color.rgb = pow(frag_color.rgb, vec3(1.0f/gamma));
		out_color = vec4(frag_color.rgb, 1.0);
	}
	else if (draw_buffer == 6) { //Draw only the downsample buffer
		out_color = textureLod(downsample_buffer, uv, bloom_mip_level);
	}
	else if (draw_buffer == 7) { //Draw only the upsample buffer
		out_color = textureLod(upsample_buffer, uv, bloom_mip_level);
	}
}