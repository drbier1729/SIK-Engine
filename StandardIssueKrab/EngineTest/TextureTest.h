#pragma once

class TextureTest : public Test {
public:
	/*
	* Sets up the Texture test.
	* Initializes the resource manager pointer
	* Returns: void
	*/
	void Setup(EngineExport* _p_engine_export_struct) override;

	/*
	* Runs the Texture test
	* Steps:
	* 1) Load a texture
	* 2) Check size of memory resource
	* 3) Load the same texture again
	* 4) Verify that the handle for the same texture was returned
	* 5) Verify that size of memory resource doesn't increase
	* 6) Load a different texture
	* 7) Check size of memory resource
	* 8) Get texture 1
	* 9) Verify that the handle is the same
	* 10) Get texture 2
	* 11) Verify that the handle is the same
	* 12) Delete all textures
	* 13) Verify size of memory resource is 0
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Deletes all the textures
	* Returns: void
	*/
	void Teardown() override;
};

