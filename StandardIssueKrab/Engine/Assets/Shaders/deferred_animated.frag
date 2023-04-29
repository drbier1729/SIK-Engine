#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D height;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 view_pos;
uniform Material material;
uniform DirectionalLight light;

in vec3 world_pos;
in vec2 tex_coords;
in mat3 TBN;

out vec4 FragColor;

layout(location = 0) out vec4 out_world_pos;
layout(location = 1) out vec4 out_world_norm;
layout(location = 2) out vec4 out_diffuse_color;
layout(location = 3) out vec4 out_specular_color;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction);

void main()
{
    vec3 N = normalize(texture(material.normal, tex_coords).rgb);
    N = (N * 2.0) - 1.0;
    N = normalize(TBN * N);

    vec3 V = normalize(view_pos - world_pos);

    vec3 result = CalculateDirectionalLight(light, N, V);

    FragColor = vec4(result*2, 1.0f);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 view_direction) {
    vec3 light_dir = normalize(-light.direction);
    
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, tex_coords).rgb;
    
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, tex_coords).rgb;
    
    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_direction, reflect_dir), 0.0f), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, tex_coords).rgb;
    
    // combine
    return (ambient + diffuse + specular);
}