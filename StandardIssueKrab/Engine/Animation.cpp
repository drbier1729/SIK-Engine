#include "stdafx.h"

#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "SkinnedMesh.h"
#include "Model.h"

#include "Animation.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "AssimpHelper.h"

Animation::Animation():
	duration{ 0.0f },
	ticks_per_sec{ 0.0f } {
}

void Animation::LoadAnimation(String const anim_name_, Model* model) {
	Assimp::Importer importer;
	String anim_path = "..\\Resources\\Models\\" + anim_name_;
	aiScene const* scene = importer.ReadFile(anim_path.c_str(), aiProcess_Triangulate |
																aiProcess_GenSmoothNormals |
																aiProcess_FlipUVs |
																aiProcess_CalcTangentSpace);
	SIK_ASSERT(scene && scene->mRootNode, "");
	aiAnimation* p_anim = scene->mAnimations[0];
	duration = static_cast<Float32>(p_anim->mDuration);
	ticks_per_sec = static_cast<Float32>(p_anim->mTicksPerSecond);
	aiMatrix4x4 global_transform = scene->mRootNode->mTransformation;
	global_transform = global_transform.Inverse();
	ReadHierarchyData(root_node, scene->mRootNode);
	ReadMissingBones(p_anim, *model);
}

Bone* Animation::FindBone(String const name) {
	auto it = std::find_if(bones.begin(), bones.end(),
		[&](Bone const& bone) {
			return bone.GetBoneName() == name;
		}
	);
	if (it == bones.end()) {
		return nullptr;
	}
	else {
		return &(*it);
	}
}

Float32 Animation::GetTicksPerSecond() const {
	return ticks_per_sec;
}

Float32 Animation::GetDuration() const {
	return duration;
}

AssimpNodeData const& Animation::GetRootNode() const {
	return root_node;
}

UnorderedMap<String, BoneInfo> const& Animation::GetBoneInfoMap() const {
	return bone_info_map;
}

void Animation::ReadMissingBones(aiAnimation const* anim, Model& model) {
	Uint32 size = anim->mNumChannels;

	UnorderedMap<String, BoneInfo> bone_info_map_local = model.GetBoneInfoMap();
	Uint32 bone_count = model.GetBoneCount();

	for (Uint32 i = 0; i < size; ++i) {
		auto channel = anim->mChannels[i];
		String bone_name{ channel->mNodeName.data };
		if (bone_info_map_local.find(bone_name) == bone_info_map_local.end()) {
			bone_info_map_local[bone_name].id = bone_count;
			model.SetBoneCount(++bone_count);
		}
		bones.push_back(Bone{ bone_name, bone_info_map_local[bone_name].id, channel });
	}
	bone_info_map = bone_info_map_local;
}

// store the information in hierarchically stored nodes
void Animation::ReadHierarchyData(AssimpNodeData& dest, aiNode const* src) {
	SIK_ASSERT(src, "");

	// update node data
	dest.name = src->mName.data;
	dest.transform = AssimpHelper::Mat4Cast(src->mTransformation);
	dest.children_count = src->mNumChildren;
	dest.children.reserve(src->mNumChildren);

	// recursively read node data for children
	for (Uint32 i = 0; i < src->mNumChildren; ++i) {
		AssimpNodeData new_node_data;
		ReadHierarchyData(new_node_data, src->mChildren[i]);
		dest.children.push_back(new_node_data);
	}
}