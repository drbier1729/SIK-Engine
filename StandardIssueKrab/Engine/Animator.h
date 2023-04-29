#pragma once

// forward declaration
class Animation;
struct AssimpNodeData;

static constexpr Uint32 MAX_BONES = 100;

class Animator {
public:
	Animator();

	void UpdateAnimation(Float32 dt);

	// load new animation
	void NewAnimation(Animation* p_anim);

	// update values of each bone
	void CalculateBoneTransform(AssimpNodeData const* node, Mat4 parent_transform);

	Vector<Mat4> const& GetFinalBoneMatrices() const;
	Vector<Tuple<Uint32, Vec3>> const& GetBonePositions() const;

public:
	// current animation
	Animation* curr_anim;

private:
	// final transforms of each bone
	Vector<Mat4> final_bone_matrices;
	// position of bones along with "set" number
	Vector<Tuple<Uint32, Vec3>> bone_positions;
	
	Float32 curr_time;

	// to keep track bone "sets"
	// needed to remove unnecessary lines between bones
	Uint32 set_counter;
};