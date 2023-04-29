#pragma once

struct BoneInfo {
	// index in final_bone_matrices
	Uint32 id;
	// mat to convert vertex from model space to bone space
	Mat4 offset;
};