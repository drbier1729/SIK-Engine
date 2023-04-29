#version 330 core

uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float glossiness;

uniform int has_color_tex;
uniform sampler2D base_color_tex;
uniform int has_normal_tex;
uniform sampler2D normal_map_tex;
uniform sampler2D shadow_map;

in vec3 world_pos;
in vec3 world_norm;
in vec2 tex_coords;
in vec4 shadow_pos;

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

    float HN = max(dot(H, N), 0.0);
    float LH = max(dot(L, H), 0.0);

    vec3 Kd = diffuse_color;

    if (has_color_tex == 1) {
		Kd = (texture(base_color_tex, vec2(tex_coords.x, 1-tex_coords.y))).xyz;
	}
       
    vec3 Ks = specular_color;
    float alpha = glossiness;

    vec3 Ii = light_color;
    vec3 Ia = vec3(0.1);
    float LN = max(dot(L, N), 0.0);

	vec3 ambient = vec3(0.1) * Kd;

	bool in_shadow = false;

    float shadow_bias = 0.005*tan(acos(clamp(LN, 0, 1)));
    shadow_bias = clamp(shadow_bias, 0,0.01);

	vec2 shadow_index = shadow_pos.xy/shadow_pos.w;    

    if (shadow_pos.w > 0 && InNormalRange(shadow_index.x) && InNormalRange(shadow_index.y))
    {
        float light_depth = texture2D(shadow_map, shadow_index).w;
        float pixel_depth = shadow_pos.w;

        in_shadow = light_depth < (pixel_depth - shadow_bias);
    }


    float diff = LN;
	vec3 diffuse = (diff * Kd) * light_color;
    float spec = pow(HN, alpha);
	vec3 specular = (Ks * spec) * light_color; 

    if (in_shadow)
    {
        FragColor = vec4(ambient, 1);
    }
    else
    {
	    FragColor =  vec4((ambient + diffuse + specular), 1.0);
    }

	// -------------------------------------------
}