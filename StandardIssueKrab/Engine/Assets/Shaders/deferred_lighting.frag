#version 330

const float pi = 3.1415926538f;

uniform sampler2D g_buffer_world_pos;
uniform sampler2D g_buffer_world_norm;
uniform sampler2D g_buffer_diffuse_color;
uniform sampler2D g_buffer_specular_color;
uniform sampler2D g_buffer_emission_color;

uniform sampler2D shadow_map;

uniform uint buffer_width, buffer_height;

uniform float min_depth, max_depth;

uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 view_pos;

uniform mat4 shadow_mat;

in vec4 vertex;

layout(location = 0) out vec4 render_buffer;

bool InNormalRange(float val)
{
    return (val >= 0.0f && val <= 1.0f);
}

void main()
{      
     //Following lines of code all read in values from the gbuffer
    //=================================================================
    vec2 uv = gl_FragCoord.xy / vec2(buffer_width, buffer_height);
    vec4 world_pos = texture(g_buffer_world_pos, uv);\

    //The normal and draw_id are stored in the same buffer to reduce lookups
    vec4 normal_id = texture(g_buffer_world_norm, uv);
    vec3 normal_vec = normal_id.xyz;
    int draw_id = int(normal_id.w);

    vec3 Kd = texture(g_buffer_diffuse_color, uv).xyz;
    vec4 spec_shini = texture(g_buffer_specular_color, uv);
    vec3 Ks = spec_shini.xyz;
    float glossiness = spec_shini.w;
    
    vec3 emission_color = texture(g_buffer_emission_color, uv).xyz;
    //=================================================================

    //Check if background is being drawn
    if (draw_id == 1 || draw_id == 2){
    //DrawSkydome
        render_buffer = vec4(Kd, 1);
        return;
    }

    //Convert colors into linear color space for calculations
    Kd = pow(Kd, vec3(2.2));
    Ks = pow(Ks, vec3(2.2));

    vec3 N = normalize(normal_vec);
    vec3 L = normalize(light_dir);
    vec3 V = normalize(view_pos - world_pos.xyz);
    vec3 H = normalize(L + V);

    vec3 Ii = light_color;
    vec3 Ia = vec3(0.1);

    float LN = max(dot(L, N), 0.0);
    float HN = max(dot(H, N), 0.0);
    float LH = max(dot(L, H), 0.0);

    //Shadow calculations
    //====================================================================
    bool in_shadow = false;

    float shadow_bias = 0.005*tan(acos(clamp(LN, 0, 1)));
    shadow_bias = clamp(shadow_bias, 0,0.01);

    vec4 shadow_pos = shadow_mat * vec4(world_pos.xyz, 1.0f);
	vec2 shadow_index = shadow_pos.xy/shadow_pos.w;

    if (shadow_pos.w > 0 && InNormalRange(shadow_index.x) && InNormalRange(shadow_index.y))
    {
        float light_depth = texture2D(shadow_map, shadow_index).w;
        float pixel_depth = shadow_pos.w;
        in_shadow = light_depth < (pixel_depth - shadow_bias);
    }
    //====================================================================
    
    if (in_shadow)
    {
        render_buffer.xyz = ((Ia/5) * Kd);
    }
    else 
    {
        float ag = sqrt(2 / (glossiness + 2));
        float a_g2 = ag * ag; //2 / (alpha + 2);

        // Phong
        float d = (HN * HN) * (a_g2 - 1) + 1;
        float D = a_g2 / (pi * d * d);

        vec3 F = Ks + ((1.0 - Ks) * pow((1.0 - LH), 5));
        float Vis = 1.0 / (LH * LH);
        
        // BRDF w/view term approximation
        vec3 BRDF = (Kd / pi) + ((F * D * Vis) / 4);

        render_buffer = vec4((Ia * Kd) + (Ii * LN * BRDF), 1); 
    }

    render_buffer += vec4(emission_color, 1);
}
