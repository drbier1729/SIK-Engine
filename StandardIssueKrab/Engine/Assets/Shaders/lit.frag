#version 330 core

uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform sampler2D shadow_map;
uniform mat4 shadow_mat;

uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float glossiness;

uniform int has_color_tex;
uniform sampler2D base_color_tex;
uniform int has_normal_tex;
uniform sampler2D normal_map_tex;

uniform float min_depth, max_depth;

in vec3 world_pos;
in vec3 world_norm;
in vec2 tex_coords;

out vec4 FragColor;

bool InNormalRange(float val)
{
    return (val >= 0.0f && val <= 1.0f);
}

void main()
{
	vec3 N = normalize(world_norm);
    vec3 L = normalize(light_dir);
    vec3 V = normalize(view_pos - world_pos);
    vec3 H = normalize(L + V);

    vec3 Kd = diffuse_color;
    vec3 Ks = specular_color;
    float alpha = glossiness;

    if (has_color_tex == 1) {
		Kd = (texture(base_color_tex, vec2(tex_coords.x, 1-tex_coords.y))).xyz;
	}

    vec3 Ii = light_color;
    vec3 Ia = vec3(0.1);
    float LN = max(dot(L, N), 0.0);
    float HN = max(dot(H, N), 0.0);
    float LH = max(dot(L, H), 0.0);

    float pi = 3.1415926538;


	bool in_shadow = false;

    float shadow_bias = 0.005*tan(acos(clamp(LN, 0, 1)));
    shadow_bias = clamp(shadow_bias, 0,0.01);

    vec4 shadow_pos = shadow_mat * vec4(world_pos, 1.0f);

	vec2 shadow_index = shadow_pos.xy/shadow_pos.w;
    if (shadow_pos.w > 0 && InNormalRange(shadow_index.x) && InNormalRange(shadow_index.y))
    {
        float light_depth = texture2D(shadow_map, shadow_index).w;
        float pixel_depth = shadow_pos.w;
        in_shadow = light_depth < (pixel_depth - shadow_bias);
    }

    if (in_shadow)
    {
        FragColor.xyz = (Ia * Kd);
    }
    else
    {
        float ag = sqrt(2 / (alpha + 2));
        float a_g2 = ag * ag; //2 / (alpha + 2);

        // Phong
        float d = (HN * HN) * (a_g2 - 1) + 1;
        float D = a_g2 / (pi * d * d);

        vec3 F = Ks + ((1.0 - Ks) * pow((1.0 - LH), 5));
        float Vis = 1.0 / (LH * LH);
        
        // BRDF w/view term approximation
        vec3 BRDF = (Kd / pi) + ((F * D * Vis) / 4);

        FragColor = vec4((Ia * Kd) + (Ii * LN * BRDF), 1); 
    }

	// -------------------------------------------

//	float spec_strength = 0.5;
//
//    vec3 light_vec = normalize(world_pos - light_dir);
//	vec3 view_vec = normalize(view_pos - world_pos);
//	vec3 reflect_vec = reflect(-light_vec, normalize(world_norm));
//
//	vec3 ambient = vec3(0.1);
//
//	vec4 tex_color = vec4(1.0);
//
//	if (texture_enabled == 1) {
//		tex_color = texture(texture_sampler, vec2(tex_coords.x, 1-tex_coords.y));
//		FragColor = tex_color;
//		return;
//	}
//
//	float diff = max(dot(normalize(world_norm), light_vec),0);
//	vec3 diffuse = diff * light_color * tex_color.xyz;
//
//	float spec = pow(max(dot(view_vec, reflect_vec), 0.0), 32);
//	vec3 specular = spec_strength * spec * light_color; 
//
//	FragColor =  vec4((ambient + diffuse + specular) * vec3(tex_coords, 0.0), 1.0);
}