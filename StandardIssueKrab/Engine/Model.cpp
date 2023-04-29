#include "stdafx.h"

#include "AnimationData.h"
#include "SkinnedMesh.h"
#include "Texture.h"
#include "VQS.h"
#include "Bone.h"

#include "Model.h"

#include "AssimpHelper.h"
#include "ResourceManager.h"
#include "GraphicsManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

Model::Model(const char* model_name):
	bone_count{ 0 } {
	LoadModel(model_name);
}

void Model::LoadAnimation(const char* anim_name) {
	// look for animation
	for (auto& i : animations) {
		if (anim_name == i.anim_name) {
			animator.NewAnimation(&i);
			return;
		}
	}

	// animation has not been loaded yet
	Animation animation{};
	animation.LoadAnimation(String{ anim_name }, this);
	animations.push_back(animation);

	animator.NewAnimation(&animations.back());
}

void Model::Update(Float32 dt) {
	animator.UpdateAnimation(dt);
}

// draw model and all its skinned meshes
void Model::Draw(GLuint shader_program) {
	// final transformations of all bones
	Vector<Mat4> const& transforms = animator.GetFinalBoneMatrices();
	for (Uint32 i = 0; i < transforms.size(); ++i) {
		String mat_loc = "final_bone_matrices[";
		mat_loc += std::to_string(i) + "]";
		p_graphics_manager->SetUniform(shader_program, transforms[i], mat_loc.c_str());
	}

	Uint32 meshes_size = static_cast<Uint32>(meshes.size());
	for (Uint32 i = 0; i < meshes_size; ++i) {
		meshes[i].Draw(shader_program);
	}
}

UnorderedMap<String, BoneInfo>& Model::GetBoneInfoMap() {
	return bone_info_map;
}

Uint32 Model::GetBoneCount() const {
	return bone_count;
}

void Model::SetBoneCount(Uint32 new_bone_count) {
	bone_count = new_bone_count;
}

void Model::ResetVertexBoneData(Vertex& vertex) {
	for (Uint32 i = 0; i < MAX_BONE_INFLUENCE; ++i) {
		vertex.bone_ids[i] = -1;
		vertex.weights[i] = 0.0f;
	}
}

void Model::SetVertexBoneData(Vertex& vertex, Int32 bone_id, Float32 weight) {
	for (Uint32 i = 0; i < MAX_BONE_INFLUENCE; ++i) {
		if (vertex.bone_ids[i] < 0) {
			vertex.bone_ids[i] = bone_id;
			vertex.weights[i] = weight;
			break;
		}
	}
}

void Model::ExtractBoneWeightForVertices(Vector<Vertex>& vertices, aiMesh* mesh, aiScene const* scene) {
	for (Uint32 bone_idx = 0; bone_idx < mesh->mNumBones; ++bone_idx) {
		Int32 bone_id = -1;
		const char* bone_name = mesh->mBones[bone_idx]->mName.C_Str();
		if (bone_info_map.find(bone_name) == bone_info_map.end()) {
			BoneInfo new_bone_info{};
			new_bone_info.id = bone_count;
			new_bone_info.offset = AssimpHelper::Mat4Cast(mesh->mBones[bone_idx]->mOffsetMatrix);
			bone_info_map[bone_name] = new_bone_info;
			bone_id = bone_count;
			++bone_count;
		}
		else {
			bone_id = bone_info_map[bone_name].id;
		}
		SIK_ASSERT(bone_id != -1, "");

		aiVertexWeight* p_weights = mesh->mBones[bone_idx]->mWeights;
		Uint32 num_weights = mesh->mBones[bone_idx]->mNumWeights;
		for (Uint32 weight_idx = 0; weight_idx < num_weights; ++weight_idx) {
			Uint32 vertex_id = p_weights[weight_idx].mVertexId;
			Float32 weight = p_weights[weight_idx].mWeight;
			SIK_ASSERT(vertex_id < vertices.size(), "");
			SetVertexBoneData(vertices[vertex_id], bone_id, weight);
		}
	}
}

// load model that has an assimp supported extension
void Model::LoadModel(String const& model_name) {
	Assimp::Importer importer;
	String model_path = "..\\Resources\\Models\\" + model_name;
	aiScene const* scene = importer.ReadFile(model_path.c_str(),	aiProcess_Triangulate |
																	aiProcess_GenSmoothNormals |
																	aiProcess_FlipUVs |
																	aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		SIK_ERROR("ASSIMP error: {}", importer.GetErrorString());
		return;
	}

	// recursively process root node
	ProcessNode(scene->mRootNode, scene);
}

// recursively process nodes
// process individual mesh in node and repeat for children nodes
void Model::ProcessNode(aiNode* node, aiScene const* scene) {
	for (Uint32 i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}
	// process children nodes
	for (Uint32 i = 0; i < node->mNumChildren; ++i) {
		ProcessNode(node->mChildren[i], scene);
	}
}

SkinnedMesh Model::ProcessMesh(aiMesh* mesh, aiScene const* scene) {
	Vector<Vertex> vertices;
	Vector<Uint32> indices;
	Vector<Texture*> textures;

	// loop through each vertex of mesh
	for (Uint32 i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		ResetVertexBoneData(vertex);

		// position
		vertex.position = AssimpHelper::Vec3Cast(mesh->mVertices[i]);
		// normal
		vertex.normal = AssimpHelper::Vec3Cast(mesh->mNormals[i]);

		// texture coords
		if (mesh->HasTextureCoords(0)) {
			Vec2 temp_vec2{};
			temp_vec2.x = mesh->mTextureCoords[0][i].x;
			temp_vec2.y = mesh->mTextureCoords[0][i].y;
			vertex.tex_coords = temp_vec2;
			// tangent
			Vec3 temp_vec3{};
			temp_vec3.x = mesh->mTangents[i].x;
			temp_vec3.y = mesh->mTangents[i].y;
			temp_vec3.z = mesh->mTangents[i].z;
			vertex.tangent = temp_vec3;
			// bi tangent
			temp_vec3.x = mesh->mBitangents[i].x;
			temp_vec3.y = mesh->mBitangents[i].y;
			temp_vec3.z = mesh->mBitangents[i].z;
			vertex.bi_tangent = temp_vec3;
		}
		else {
			vertex.tex_coords = Vec2{ 0.0f, 0.0f };
		}
		vertices.push_back(vertex);
	}

	// loop through each face of mesh and retrieve indices
	for (Uint32 i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (Uint32 j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	// TODO: only one material in shader currently
	// diffuse maps
	Vector<Texture*> diffuse_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
	textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
	// specular maps
	Vector<Texture*> specular_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
	textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
	// normal maps
	Vector<Texture*> normal_maps = LoadMaterialTextures(material, aiTextureType_NORMALS, "normal");
	textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
	// height maps
	Vector<Texture*> height_maps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "height");
	textures.insert(textures.end(), height_maps.begin(), height_maps.end());

	// bone weight
	ExtractBoneWeightForVertices(vertices, mesh, scene);

	return SkinnedMesh{ vertices, indices, textures };
}

Vector<Texture*> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type, const char* type_name) {
	Vector<Texture*> textures;

	for (Uint32 i = 0; i < material->GetTextureCount(type); ++i) {
		aiString str;
		material->GetTexture(type, i, &str);

		// skip if texture was loaded before
		Bool skip = false;
		for (Uint32 j = 0; j < loaded_textures.size(); ++j) {
			if (loaded_textures[j]->texture_path == String{ str.C_Str() }) {
				textures.push_back(loaded_textures[j]);
				skip = true;
				break;
			}
		}

		// texture has not been loaded
		if (!skip) {
			String texture_path = String{ "..\\Resources\\Textures\\" } + String{ str.C_Str() };
			Texture* texture = p_resource_manager->LoadTexture(str.C_Str());
			texture->texture_type = String{ type_name };
			texture->texture_path = String{ texture_path.c_str() };
			textures.push_back(texture);
			loaded_textures.push_back(texture);
		}
	}

	return textures;
}
