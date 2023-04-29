#include "stdafx.h"

#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "Animation.h"

#include "Animator.h"


Animator::Animator() :
	curr_anim{ nullptr },
	curr_time{ 0.0f },
	set_counter{ 0 } {
	// MAX_BONES = 100 from vertex shader
	bone_positions.reserve(MAX_BONES);
	final_bone_matrices.reserve(MAX_BONES);
	for (Uint32 i = 0; i < MAX_BONES; ++i) {
		final_bone_matrices.push_back(Mat4{ 1.0f });
	}
}

void Animator::UpdateAnimation(Float32 dt) {
	if (curr_anim) {
		curr_time += curr_anim->GetTicksPerSecond() * dt;
		curr_time = fmod(curr_time, curr_anim->GetDuration());
		bone_positions.clear();
		set_counter = 0;
		CalculateBoneTransform(&curr_anim->GetRootNode(), Mat4{ 1.0f });
	}
}

void Animator::NewAnimation(Animation* p_anim) {
	curr_anim = p_anim;
	curr_time = 0.0f;
}

void Animator::CalculateBoneTransform(AssimpNodeData const* node, Mat4 parent_transform) {
	String node_name = node->name;
	Mat4 node_transform = node->transform;

	Bone* bone = curr_anim->FindBone(node_name);
	if (bone) {
		// get local transform of each bone
		bone->Update();
		node_transform = bone->GetLocalTransform();
	}

	// transform wrt world
	Mat4 global_transform = parent_transform * node_transform;

	UnorderedMap<String, BoneInfo> const& bone_info_map = curr_anim->GetBoneInfoMap();
	if (bone_info_map.find(node_name) != bone_info_map.end()) {
		Uint32 index = bone_info_map.at(node_name).id;
		Mat4 offset = bone_info_map.at(node_name).offset;
		final_bone_matrices[index] = global_transform * offset;

		// bone positions for drawing skeleton
		bone_positions.push_back(std::make_tuple(set_counter, global_transform * Vec4{ 0.0f, 0.0f, 0.0f, 1.0f }));
	}

	// update transforms of all children recursively
	for (Uint32 i = 0; i < node->children_count; ++i) {
		CalculateBoneTransform(&node->children[i], global_transform);
	}

	// increment "set" of bones
	++set_counter;
}

Vector<Mat4> const& Animator::GetFinalBoneMatrices() const {
	return final_bone_matrices;
}

Vector<Tuple<Uint32, Vec3>> const& Animator::GetBonePositions() const {
	return bone_positions;
}