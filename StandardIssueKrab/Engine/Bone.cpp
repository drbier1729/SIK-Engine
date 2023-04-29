#include "stdafx.h"

#include "VQS.h"

#include "Bone.h"

#include "AssimpHelper.h"

Bone::Bone(String _name, Uint32 _id, aiNodeAnim* channel) :
	name{ _name },
	id{ _id },
	local_transform{ 1.0f },
	num_key_frames{ channel->mNumPositionKeys },
	num_incr{ 2.0f },
	itr{ 0 },
	keyframe_index{ 0 },
	curr_vqs{},
	prev_vqs{} {
	// initializing keyframe data
	vqs_key_frames.reserve(num_key_frames);
	for (Uint32 i = 0; i < num_key_frames; ++i) {
		aiVector3D ai_pos = channel->mPositionKeys[i].mValue;
		aiQuaternion ai_rot = channel->mRotationKeys[i].mValue;
		aiVector3D ai_scale = channel->mScalingKeys[i].mValue;
		VQS vqs;
		vqs.v = AssimpHelper::Vec3Cast(ai_pos);
		vqs.q = AssimpHelper::QuatCast(ai_rot);
		vqs.s = AssimpHelper::Vec3Cast(ai_scale);
		vqs_key_frames.push_back(vqs);
	}

	// incremental VQS preprocessing
	VQS_c.reserve(num_key_frames);
	for (Uint32 i = 0; i < num_key_frames - 1; ++i) {
		// for each pair of keyframes
		VQS vqs_c;

		// position v_c = (v_n - v_0) / n
		aiVector3D v_c = (channel->mPositionKeys[i + 1].mValue - channel->mPositionKeys[i].mValue) / num_incr;
		vqs_c.v = AssimpHelper::Vec3Cast(v_c);

		// rotation q_c = [cos(beta), sin(beta)v] (iSlerp)
		// q_0 dot q_n = cos(alpha), beta = alpha / n
		// v = ( (s_0 v_n) - (s_n v_0) + (v_0 cross v_n) ) / sin(alpha)
		Quat q_0 = AssimpHelper::QuatCast(channel->mRotationKeys[i].mValue);
		q_0 = glm::normalize(q_0);
		Quat q_n = AssimpHelper::QuatCast(channel->mRotationKeys[i + 1].mValue);
		q_n = glm::normalize(q_n);
		Float32 q_0_Dot_q_n = glm::dot(q_0, q_n);
		q_0_Dot_q_n = glm::clamp(q_0_Dot_q_n, -0.99f, 0.99f);
		Float32 alpha = acosf(q_0_Dot_q_n);
		Float32 beta = alpha / num_incr;
		Vec3 v =	sinf(beta) * ((q_0.w * Vec3{ q_n.x, q_n.y, q_n.z }) - (q_n.w * Vec3{ q_0.x, q_0.y, q_0.z }) +
					(glm::cross(Vec3{ q_0.x, q_0.y, q_0.z }, Vec3{ q_n.x, q_n.y, q_n.z }))) / sinf(alpha);
		vqs_c.q = Quat{ cosf(beta), v };

		// scale s_c = (s_n / s_0) ^ (1 / n)
		vqs_c.s.x = powf((channel->mScalingKeys[i + 1].mValue.x / channel->mScalingKeys[i].mValue.x), (1.0f /num_incr));
		vqs_c.s.y = powf((channel->mScalingKeys[i + 1].mValue.y / channel->mScalingKeys[i].mValue.y), (1.0f /num_incr));
		vqs_c.s.z = powf((channel->mScalingKeys[i + 1].mValue.z / channel->mScalingKeys[i].mValue.z), (1.0f /num_incr));

		VQS_c.push_back(vqs_c);
	}
}

void Bone::Update() {
	// first iteration between keyframes
	if (itr == 0) {
		curr_vqs = vqs_key_frames[keyframe_index];
	}
	// iterations within keyframe finished
	if (itr > num_incr) {
		itr = 0;
		keyframe_index = (keyframe_index + 1) % (num_key_frames - 1);
		curr_vqs = vqs_key_frames[keyframe_index];
	}
	else {
		// v_curr = v_c + v_prev
		curr_vqs.v = VQS_c[keyframe_index].v + prev_vqs.v;
		// q_curr = q_c * q_prev
		curr_vqs.q = VQS_c[keyframe_index].q * prev_vqs.q;
		// s_curr = s_c * s_prev
		curr_vqs.s = VQS_c[keyframe_index].s * prev_vqs.s;
	}
	prev_vqs = curr_vqs;
	++itr;

	// updating transform matrix
	Mat4 translation_mat = glm::translate(Mat4{ 1.0f }, curr_vqs.v);
	Quat rot = curr_vqs.q;
	rot = glm::normalize(rot);
	Mat4 rotation_mat = Mat4{ rot };
	Mat4 scale_mat = glm::scale(Mat4{ 1.0f }, Vec3{ curr_vqs.s });

	local_transform = translation_mat * rotation_mat * scale_mat;
}

Mat4 Bone::GetLocalTransform() const {
	return local_transform;
}

Uint32 Bone::GetBoneID() const {
	return id;
}

String Bone::GetBoneName() const {
	return name;
}
