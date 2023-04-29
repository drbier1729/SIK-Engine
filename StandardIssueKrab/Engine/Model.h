#pragma once

#include "Animation.h"
#include "Animator.h"

// forward declaration
class Texture;

class Model {
public:
	Vector<Animation> animations;
	Animator animator;
	//Vector<Animator> animators;
	Vector<Texture*> loaded_textures;
	Vector<SkinnedMesh> meshes;
	String directory;

	Model(const char* model_name);

	void LoadAnimation(const char* anim_name);
	void Update(Float32 dt);

	void Draw(GLuint shader_program);

	UnorderedMap<String, BoneInfo>& GetBoneInfoMap();
	Uint32 GetBoneCount() const;
	void SetBoneCount(Uint32 new_bone_count);

private:
	UnorderedMap<String, BoneInfo> bone_info_map;
	Uint32 bone_count;

	void ResetVertexBoneData(Vertex& vertex);
	void SetVertexBoneData(Vertex& vertex, Int32 bone_id, Float32 weight);
	void ExtractBoneWeightForVertices(Vector<Vertex>& vertices, aiMesh* mesh, aiScene const* scene);

	void LoadModel(String const& model_name);
	void ProcessNode(aiNode* node, aiScene const* scene);
	SkinnedMesh ProcessMesh(aiMesh* mesh, aiScene const* scene);
	Vector<Texture*> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const char* type_name);
};