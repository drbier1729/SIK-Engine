#version 330 core

#define PI 3.1415926538

uniform mat4 view_inverse;

uniform vec3 local_light_color;
uniform vec3 local_light_pos;
uniform float local_light_radius;

uniform sampler2D g_buffer_world_pos;
uniform sampler2D g_buffer_world_norm;
uniform sampler2D g_buffer_diffuse_color;
uniform sampler2D g_buffer_specular_color;

uniform uint buffer_width, buffer_height;

uniform float gamma;
uniform int debug_local_lights;
uniform int ease_out;

in vec3 in_pos;

void main() {
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
    
    //=================================================================

    //If the object in the gbuffer is the background then do nothing
    if (draw_id == 1 || draw_id == 2)
        return;

    vec3 eye_pos = (view_inverse*vec4(0, 0, 0, 1)).xyz;

    vec3 outColor = vec3(0.0f);

    //Check if the world position corresponding to this pixel 
    // is in range of this particular local light
    vec3 local_light_vec = local_light_pos - world_pos.xyz;
    float light_distance_squared = dot(local_light_vec, local_light_vec);
    float local_radius_squared = pow(local_light_radius, 2);
    if (light_distance_squared < local_radius_squared) {     
        //Convert colors into linear color space for calculations
        Kd = pow(Kd, vec3(2.2));
        Ks = pow(Ks, vec3(2.2));
        vec3 Lc = pow(local_light_color, vec3(2.2));
        //Inside of local light influence. Do additive local light caclulation
        vec3 N = normalize(normal_vec);
        vec3 L = normalize(local_light_vec);
        vec3 V = normalize(eye_pos - world_pos.xyz);
        vec3 H = normalize(L+V);

        float LN = max(dot(L, N), 0.0);
        float NH = max(dot(N, H), 0.0);
        float LH = max(dot(L, H), 0.0);

        vec3 brdf;
        float distribution;
        float alpha;

        vec3 fresnel = Ks + ((vec3(1, 1, 1) - Ks)*pow((1 - LH), 5));
        float visibility = 1 / (LH * LH);

        alpha =  sqrt(2 / (glossiness + 2));
        distribution = ((alpha+2)/(2*PI))*pow(NH, alpha);
            

        brdf = (Kd/PI) + ((fresnel*visibility*distribution)/4);
        outColor = Lc*LN*brdf;
        //Ease out the brightness as we move away from the center of the light
        if (ease_out == 1) {
            outColor *= max((1/light_distance_squared) - (1/local_radius_squared), 0);
        }

        outColor = max(outColor, vec3(0));

        if (debug_local_lights == 1) {
            outColor = vec3(1, 0, 0);
        }
    }
    gl_FragColor.xyz = outColor;
    gl_FragColor.w = 1;
}