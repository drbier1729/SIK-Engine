#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 in_tex_coords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bi_tangent;
layout (location = 5) in ivec4 bone_ids; 
layout (location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 final_bone_matrices[MAX_BONES];

out vec3 world_pos;
out vec2 tex_coords;
out mat3 TBN;

void main()
{
    mat4 bone_transform = mat4(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (bone_ids[i] >= 0) {
            bone_transform += final_bone_matrices[bone_ids[i]] * weights[i];
        }
    }

    mat3 normal_matrix = transpose(inverse(mat3(bone_transform)));

    vec3 T = normalize(normal_matrix * tangent);
    vec3 B = normalize(normal_matrix * bi_tangent);
    vec3 N = normalize(normal_matrix * normal);

    TBN = mat3(T, B, N);
    tex_coords = in_tex_coords;
    world_pos = vec3(model * vec4(position, 1.0));

    gl_Position = proj * view * model * bone_transform * vec4(position, 1.0);
}