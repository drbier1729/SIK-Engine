#pragma once

class Bone {
public:
	Bone(String _name, Uint32 _id, aiNodeAnim* channel);

	void Update();

	Mat4 GetLocalTransform() const;
	Uint32 GetBoneID() const;
	String GetBoneName() const;

private:
	Uint32 num_key_frames;

	// bone data
	Mat4 local_transform;
	Uint32 id;
	String name;

	// incremental VQS preprocessed info
	Vector<VQS> VQS_c;
	Vector<Mat3> N_c;

	// number of steps between frames
	Float32 num_incr;
	// index of current iteration within keyframe
	Uint32 itr;
	// index of keyframe within animation loop
	Uint32 keyframe_index;

	// current vqs
	VQS curr_vqs;
	// prev vqs
	VQS prev_vqs;
	// all bone key frames data
	Vector<VQS> vqs_key_frames;
};