#include "stdafx.h"
#include "Test.h"
#include "MeshTest.h"
#include "Engine/MemoryResources.h"
#include "Engine/ResourceManager.h"
#include "Engine/GraphicsManager.h"


void MeshTest::Setup(EngineExport* _p_engine_export_struct) {
	p_resource_manager = _p_engine_export_struct->p_engine_resource_manager;
#ifdef STR_DEBUG
	p_dbg_string_dictionary = _p_engine_export_struct->p_dbg_string_dictionary;
#endif
	p_graphics_manager = _p_engine_export_struct->p_engine_graphics_manager;
	SetRunning();
	return;
}

/*
* Runs the Mesh test
* Steps:
* 1) Load a nonexistent mesh
* 2) load a mesh
* 3) Load the same mesh again
* 4) Check pointers to each mesh are the same
* 5) Get the mesh
* 6) Get the mesh with a string hash
* 7) Load a second mesh
* 8) Load second mesh again
* 9) Verify second meshes are the same
* 10) Delete all meshes and check if they were deleted
* Returns: TestState
*/
void MeshTest::Run() {
	SIK_INFO("1. Load Nonexistent mesh");
	Mesh* null_mesh = p_resource_manager->LoadMesh("null.fbx");
	if (null_mesh != nullptr)
	{
		SIK_ERROR("Incorrect handling of bad file path");
		SetFailed();
		return;
	}

	SIK_INFO("2. Load the mesh");
	Mesh* sphere_mesh = p_resource_manager->LoadMesh("sphere.fbx");
	if (sphere_mesh == nullptr)
	{
		SIK_ERROR("Failed to load sphere mesh");
		SetFailed();
		return;
	}

	SIK_INFO("3. Load the same mesh");
	Mesh* sphere_mesh_2 = p_resource_manager->LoadMesh("sphere.fbx");
	if (sphere_mesh_2 == nullptr)
	{
		SIK_ERROR("Failed to find loaded sphere mesh");
		SetFailed();
		return;
	}

	SIK_INFO("4. Verify meshes are the same");
	if (sphere_mesh != sphere_mesh_2)
	{
		SIK_ERROR("Sphere mesh was not properly stored");
		SetFailed();
		return;
	}

	SIK_INFO("5. Get Sphere Mesh");
	Mesh* sphere_mesh_copy = p_resource_manager->GetMesh("sphere.fbx");
	if (sphere_mesh != sphere_mesh_copy)
	{
		SIK_ERROR("Sphere mesh was not properly stored");
		SetFailed();
		return;
	}

	SIK_INFO("6. Get Sphere Mesh with string hash");
	Mesh* sphere_mesh_copy_2 = p_resource_manager->GetMesh("sphere.fbx"_sid);
	if (sphere_mesh != sphere_mesh_copy_2)
	{
		SIK_ERROR("Sphere mesh was not properly stored");
		SetFailed();
		return;
	}

	SIK_INFO("7. Load the second mesh");
	Mesh* sphere_p_mesh = p_resource_manager->LoadMesh("sphere_primitive.fbx");
	if (sphere_p_mesh == nullptr)
	{
		SIK_ERROR("Failed to load sphere mesh");
		SetFailed();
		return;
	}

	SIK_INFO("8. Load the same 2nd mesh");
	Mesh* sphere_p_mesh_2 = p_resource_manager->LoadMesh("sphere_primitive.fbx");
	if (sphere_p_mesh_2 == nullptr)
	{
		SIK_ERROR("Failed to find loaded sphere mesh");
		SetFailed();
		return;
	}

	SIK_INFO("9. Verify meshes are the same");
	if (sphere_p_mesh != sphere_p_mesh_2)
	{
		SIK_ERROR("Sphere mesh was not properly stored");
		SetFailed();
		return;
	}

	SetPassed();

}

void MeshTest::Teardown() {
	return SetPassed();
}