#include "stdafx.h"

#include "Texture.h"

#include "SkinnedMesh.h"

#include "GraphicsManager.h"

SkinnedMesh::SkinnedMesh(Vector<Vertex> _vertices, Vector<Uint32> _indices, Vector<Texture*> _textures):
	vertices{ _vertices },
	indices{ _indices },
	textures{ _textures } {
    SetupMesh();
}

void SkinnedMesh::Draw(GLuint shader_program) {
    //Uint32 diffuse_num = 1;
    //Uint32 specular_num = 1;
    //Uint32 normal_num = 1;
    //Uint32 height_num = 1;

    for (Int32 i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);

        String name = textures[i]->texture_type;

        //// retrieve texture num
        //String number;
        //if (name == "diffuse") {
        //    number = std::to_string(diffuse_num++);
        //}
        //else if (name == "specular") {
        //    number = std::to_string(specular_num++);
        //}
        //else if (name == "normal") {
        //    number = std::to_string(normal_num++);
        //}
        //else if (name == "height") {
        //    number = std::to_string(height_num++);
        //}

        // set sampler to correct texture unit
        p_graphics_manager->SetUniform(shader_program, i, ("material." + name).c_str());
        // bind texture
        glBindTexture(GL_TEXTURE_2D, textures[i]->texture_id);
    }

    // draw mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<Uint32>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // set back to default
    glActiveTexture(GL_TEXTURE0);
}

void SkinnedMesh::SetupMesh() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // load data into vertex buffers
    // structs have sequetial memory layout
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Uint32), indices.data(), GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // texture coords attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    // tangent attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // bi tangent attribute
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bi_tangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, bone_ids));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

    glBindVertexArray(0);
}