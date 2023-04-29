#include "stdafx.h"
#include "MeshRenderer.h"
#include "GameObject.h"
#include "Mesh.h"
#include "Material.h"
#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "SkinnedMesh.h"
#include "Model.h"

void MeshRenderer::Use() {
	material->Use();
	if (mesh != nullptr) {
		mesh->Use();
	}
}

void MeshRenderer::Unuse() {
	if (mesh != nullptr) {
		mesh->Unuse();
	}
	material->Unuse();
}

void MeshRenderer::Draw() {
	if (not enabled)
		return;


	if (mesh != nullptr) {
		mesh->Draw();
	}
}
