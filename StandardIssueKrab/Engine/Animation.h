#pragma once

// forward declarations
class Model;

// data structure to hierarchically store node data
struct AssimpNodeData {
	Mat4 transform{};
	String name;
	Uint32 children_count{};
	Vector<AssimpNodeData> children;
};

class Animation {
private:
	Float32 duration;
	Float32 ticks_per_sec;
	AssimpNodeData root_node;
	UnorderedMap<String, BoneInfo> bone_info_map;
	Vector<Bone> bones;

private:
	void ReadMissingBones(aiAnimation const* anim, Model& model);
	void ReadHierarchyData(AssimpNodeData& dest, aiNode const* src);

public:
	const char* anim_name;

public:
	Animation();

	void LoadAnimation(String const anim_name, Model* model);

	Bone* FindBone(String const name);

	Float32 GetTicksPerSecond() const;
	Float32 GetDuration() const;
	AssimpNodeData const& GetRootNode() const;
	UnorderedMap<String, BoneInfo> const& GetBoneInfoMap() const;
};