#version 330 core

#define PI 3.1415926538

uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform float glossiness;
uniform vec3 emission_color;

uniform int has_color_tex;
uniform sampler2D base_color_tex;

uniform int has_normal_map;
uniform sampler2D normal_map;

//Used to determine if drawing the bg or objects
uniform int draw_id;

//Background color gradient
uniform vec3 diffuse_color_2;

in vec4 world_pos;
in vec3 world_norm;
in vec2 tex_coords;
in vec4 shadow_pos;
in vec3 eye_vec;
in vec3 tangent_vec;

layout(location = 0) out vec4 out_world_pos;
layout(location = 1) out vec4 out_world_norm;
layout(location = 2) out vec4 out_diffuse_color;
layout(location = 3) out vec4 out_specular_color;
layout(location = 4) out vec4 out_emission_color;

void main()
{
    vec3 N = normalize(world_norm);

    vec3 Kd = vec3(0.0f);
    vec3 Ks = vec3(0.0f);

    if (has_normal_map == 1) {
        vec3 delta = texture2D(normal_map, vec2(tex_coords.x, 1.0 - tex_coords.y)).xyz;
        delta = delta*2.0 - vec3(1, 1, 1);
        vec3 T = normalize(tangent_vec);
        vec3 B = normalize(cross(T, N));
        N = delta.x*T + delta.y*B + delta.z*N;
    }

    if (has_color_tex == 1) {
        //skydome drawing
        if (draw_id == 1) {
            vec3 V = normalize(eye_vec);
            vec2 tex_coords = vec2((- 1.0) * atan(V.z,V.x)/(2*PI), acos(-V.y)/PI); 	

            Kd = texture2D(base_color_tex, tex_coords).xyz;
            Kd = vec3(1.0f, 0.0f, 0.0f);
        }
        else if (draw_id == 2) {
            Kd = texture2D(base_color_tex, vec2(tex_coords.x, 1.0 - tex_coords.y)).xyz;
        }

		Kd = (texture(base_color_tex, vec2(tex_coords.x, 1-tex_coords.y))).xyz;
	}
    else {
        Kd = diffuse_color;
        Ks = specular_color;
        if (draw_id == 2) {
            float t = tex_coords.y;
            Kd = diffuse_color * (1 - t) + diffuse_color_2 * t;
        }
    }
    
    
    out_world_pos = world_pos;
    out_world_norm = vec4(N, draw_id);
    out_diffuse_color = vec4(Kd, 1);
    out_specular_color = vec4(Ks, glossiness);
    out_emission_color = vec4(emission_color, 1);
}