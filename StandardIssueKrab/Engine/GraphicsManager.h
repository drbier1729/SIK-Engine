#pragma once
/****************************************************************************** /
/* !
/* File GraphicsManager.h
/* Author Andrew Rudasics
/* Email: andrew.rudasics@digipen.edu
/* Date 9/11/2022
/* Interface for global manager to handle rendering of graphics objects.
/* DigiPen Institute of Technology © 2022
/******************************************************************************/

#include "MemoryResources.h"
#include "Mesh.h"
#include "Material.h"
#include "MeshRenderer.h"
#include "FixedObjectPool.h"
#include "FBO.h"
#include "MemoryResources.h"
#include "Texture.h"
#include "MSAA.h"
#include "GUIRenderer.h"

// TODO (Dylan) : remove
struct PhysDebugBox;
class Arrow;

struct DirectionalLight
{
	Vec3 color{};
	Vec3 position{};
	Vec3 direction{};
	Float32 intensity{};
};

struct LocalLight
{
	Vec3 position{};
	Vec3 color{};
	float radius{};
	bool enabled;
	bool ease_out;
};

struct ConeLocalLight
{
	Vec3 position{};
	Quat orientation{};
	Vec3 color{};
	float radius{};
	float length{};
	bool enabled;
	bool ease_out;
};

struct VertexArrayObject
{
	GLuint vao_id;
	GLuint vertex_buffer_id;
	GLuint index_buffer_id;
};

/*
* The Graphics Manager Class is used to manage the rendering of all game object.
* Accepts and stores a list of draw commands, executes draws with low-level API
*/
class GraphicsManager {
private:

	// TODO: Add system for storing geo, remove hardcoding

	// Shaders
	GLuint bound_shader_program;
	GLuint* shader_program, * shadow_program, * fbo_program,
		* normal_program, * gbuffer_program, * deferred_lighting_program,
		* local_lights_program, * downsample_program, * upsample_program,
		* post_process_program, * bloom_program,
		* wireframe_shader, // TODO (Dylan) : remove this
		* particle_shader, // TODO (Dylan) : remove this
		* geo_particle_shader; // TODO (Brian) : remove this

	// Camera and Window
	SDL_GLContext gl_context = NULL;
	SDL_Window* p_window = NULL;
	Uint32 window_width;
	Uint32 window_height;
	class RenderCam* active_camera;
	Mat4 ortho_proj;
	Bool invert_mouse_y = false;

	// Lights
	bool local_lights_on = false;
	Vector<LocalLight> local_lights;
	Vector<ConeLocalLight> cone_local_lights;
	Vector<DirectionalLight> m_lights;

	// Mesh Renderers
	static constexpr SizeT MAX_RENDER_OBJECTS = 8192;
	FixedObjectPool<MeshRenderer, MAX_RENDER_OBJECTS> m_renderers;


	// Shadow Map
	UniquePtr<FBO> shadow_fbo;
	Vec3 shadow_look_target;

	//Struct for the full screen quad
	VertexArrayObject full_screen_quad;

	// Debug Drawing
	Bool dbg_draw = false;
	static Bool debug_drawing_enabled;
	UniquePtr<PhysDebugBox> phys_box; // TODO (Dylan) : remove this
	UniquePtr<Arrow> arrow;			  // TODO (Brian) : remove this
	Bool wireframe_toggle;

	// Normals
	Bool normalsOn;

	// Sky Dome -- TODO (Brian) : organize or remove
	GLuint* skyShader_program;
	GLuint* bgShader_program;
	Mesh const* SkyDome;
	Mesh const* Background;
	GLuint* sky_shader_deferred;
	GLuint* bg_shader_deferred;
	Bool skyOn;
	Bool bgOn;
	Bool bgTexOn;

	// Background Colors
	Vec3 color1;
	Vec3 color2;

	// GUI Renderers
	static constexpr SizeT MAX_GUI_RENDER_OBJECTS = 1024;
	FixedObjectPool<GUIRenderer, MAX_GUI_RENDER_OBJECTS> gui_renderer_list;

	// MSAA
	UniquePtr<MSAA> msaa_fbo;
	Bool msaa_toggle;
	Bool vsync_toggle;
	
	// Geometic Particles
	Bool geo_particles_toggle;

	// G Buffer
	UniquePtr<FBO> gbuffer_fbo;
	
	// Bloom Effects
	UniquePtr<FBO> post_process_fbo;

	//Functions to turn the depth test On/Off
	void SetDepthTestOn();
	void SetDepthTestOff();
	
	void GUIRender();

	/*Function to bind an attribute locations
	* for the given variable
	* Returns: void
	*/
	void BindAttrib(GLuint shader_program, GLuint attrib, const std::string& var_name);

	/*Function to bind an output attribute locations
	* for the given variable
	* Returns: void
	*/
	void BindOutputAttrib(GLuint shader_program, GLuint attrib, const std::string& var_name);
	/*
	* Calls Capacity() on the underlying memory resource
	* Returns: size_t - the space left in bytes
	*/
	SizeT BufferSpaceAvailable() const;
	/*
	* Calls Size() on the underlying memory resource
	* Returns: size_t - the space left in bytes
	*/
	SizeT BufferSpaceUsed() const;

	/*
	* Sets the orthographic projection based on the window size
	* Returns: void
	*/
	void SetOrthoProj();

	/*
	* Clears an FBO. If no FBO is specified then clears the render buffer
	* Returns: void
	*/
	void ClearBuffer(Vec4 clear_color, FBO* _buffer_to_clear = nullptr);

	void EnableFrontFaceCulling();

	void EnableBackFaceCulling();

	void DisableFaceCulling();

	void EnableDepthTest();

	void DisableDepthTest();

	void EnableMultiSampling();

	void DisableMultiSampling();

	void EnableAdditiveBlend();

	void EnableAlphaBlend();

	void DisableBlend();

	/*
	* Makes it so that the following draw calls go to the render buffer
	*/
	void EnableRenderBuffer();

	/*
	* Performs a shadow pass.
	* Draws the depth of all objects into a depth buffer from the POV of the light.
	* Returns : void
	*/
	void ShadowPass();

	/*
	* Performs the lighting pass.
	* Returns : void
	*/
	void LightingPass();

	/*
	* Performs the GBuffer pass.
	* Returns : void
	*/
	void GBufferPass();

	/*
	* Performs the Deferred Lighting pass.
	* Returns : void
	*/
	void DeferredLightingPass();

	/*
	* Performs the local lights pass
	* Returns : void
	*/
	void LocalLightsPass();

	/*
	* Iterates through the list of local lights and renders them
	* Returns: void
	*/
	void DrawLocalLights();

	/*
	* A rendering pass for bloom effects
	* Returns: void
	*/
	void BloomPass();

	/*
	* A rendering pass for applying post processing effects
	* Returns: void
	*/
	void PostProcessPass();

	/*
	* Draw the normals for all the mesh renderers
	*/
	void DrawNormals();

	//Create the FBOs required by the graphics manager
	void CreateFBOs();

	//Delete all the created FBOs
	void DeleteFBOs();

	/*Create a quad that fills the entire screen.
	* Used to render the gbuffer
	* Returns: GLuint - vao_id of the quad
	*/
	VertexArrayObject GenerateFullScreenQuad();

	/*
	* Calls glDrawElements on the full screen quad vao
	* Returns: void
	*/
	void DrawFullScreenQuad();

	/*
	* Delete a created vertex array object
	*/
	void DeleteVAO(VertexArrayObject& vao);

	/*
	* Gets the shadow projection matrix
	*/
	Mat4 GetShadowProjMatrix(const Vec3& light_pos);
	Mat4 GetShadowViewMatrix(const Vec3 light_pos);
	/*
	* Gets the shadow matrix used for shadow calculation
	* Returns: Mat4 - the shadow matrix transformation
	*/
	Mat4 CalculateShadowMatrix(const Vec3& light_pos);

	/*
	* Method to create the FBO used for the bloom pass
	* Creates an FBO with 3 attachments.
	* Creates several mip levels for each attachment according to the number of
	* downsampling passes.
	* Returns: void
	*/
	void CreateBloomFBO();
public:
	enum class DrawBuffer {
		DISABLE = 0,
		WORLD_POS = 1,
		NORMALS = 2,
		DIFFUSE = 3,
		SPECULAR = 4,
		SHADOW = 5,
		BLOOM_DOWNSAMPLE = 6,
		BLOOM_UPSAMPLE = 7,
		EMISSION = 8
	};
	DrawBuffer draw_buffer = DrawBuffer::DISABLE;

	bool deferred_shading_enabled;
	Float32 gamma;
	Float32 exposure;

	//Parameters to control the bloom effects
	bool bloom_enabled;
	Float32 bloom_threshold;
	Float32 bloom_factor;
	//Using int instead of Uint8 for IMGUI sliders
	int bloom_mip_level;
	int bloom_downsample_passes;
	

	bool debug_local_lights;

	// Default Constructor
	GraphicsManager();

	// Destructor
	~GraphicsManager();

	//Teardown to clear the object pools
	void Clear();

	void Setup();
	void Update(Float32 dt);

	// Draws all objects in the scene
	void RenderScene();

	void SwapBuffers();

	bool InitWindow();
	void DestroyWindow();
	SDL_GLContext GetGLContext();
	SDL_Window* GetPWindow();

	/*
	* Returns the active render cam
	*/
	RenderCam* GetPActiveCam();

	//Sets the active render cam
	void SetPActiveCam(RenderCam* _p_cam);

	//Intialize all the required shaders for the program
	void InitShaders();

	//Binds a shader program to be the currently active shader
	void BindShaderProgram(GLuint shader_program);
	//Returns the currently bound shader program
	GLuint GetBoundShader() const;
	//Sets the bound shader program to 0
	void UnbindShaderProgram();

	//Sets an Int to the specified shader_program
	void SetUniform(GLuint shader_program, const Int32& val, const char* var_name) const;
	//Sets an unsigned Int to the specified shader_program
	void SetUniform(GLuint shader_program, const Uint32& val, const char* var_name) const;
	//Sets a float to the specified shader_program
	void SetUniform(GLuint shader_program, const Float32& val, const char* var_name) const;
	//Sets a vec3 to the specified shader_program
	void SetUniform(GLuint shader_program, const Vec3& val, const char* var_name) const;
	//Sets a vec4 to the specified shader_program
	void SetUniform(GLuint shader_program, const Vec4& val, const char* var_name) const;
	//Sets a float to the specified shader_program
	void SetUniform(GLuint shader_program, const Mat4& val, const char* var_name) const;

	/*
	* Creates a GUIRenderer object and adds it to the list
	* Returns: GUIRenderer* - pointer to the created object
	*/
	class GUIRenderer* CreateGUIRenderer(class GUIObject* _gui_obj);

	/*
	* Delete all GUI renderer objects
	* Clears the memory resources used
	* Returns: void
	*/
	void DeleteAllGUIRenderers();

	MeshRenderer* CreateMeshRenderer(Material* mat, Mesh* mesh);
	void DestroyMeshRenderer(MeshRenderer* mr);
	void CleanupDestroyedRenderers();

	inline void ToggleDebugDrawing() noexcept;
	static inline void EnableDebugDrawing() noexcept;
	static inline Bool IsDebugDrawingEnabled() noexcept { return debug_drawing_enabled; }

	/*
	* Draw the skydome. If deferred=true uses the deferred lighting pass
	* Returns: void
	*/
	void DrawSkyDome(bool deferred=false);
	void DrawBackgound(bool deferred = false);

	// msaa
	void DrawFBO(FBO* fbo, Uint8 color_attachment = 0);

	// Call this function if you want your prototype to load with skybox on
	inline void SetSkyBoxToggle() {
		skyOn = true;
		bgOn = false;
	}
	inline Bool& GetSkyToggle() { return skyOn; }
	inline Bool& GetBgToggle() { return bgOn; }
	inline Bool& GetBgTexToggle() { return bgTexOn; }
	inline Bool& GetWireframeToggle() { return wireframe_toggle; }

	inline Vec3& GetColor1() { return color1; }
	inline Vec3& GetColor2() { return color2; }
	inline Bool& GetNormalToggle() { return normalsOn; }

	inline Bool& GetMsaaToggle() { return msaa_toggle; }

	inline void SetInvertMouseY(Bool val) { invert_mouse_y = val; }
	inline Bool& GetInvertMouseY() { return invert_mouse_y; }

	inline Uint32 GetWindowWidth() { return window_width; }
	inline Uint32 GetWindowHeight() { return window_height; }

	inline Bool& GetVSyncToggle() { return vsync_toggle; }
	void ToggleVSync();

	inline Bool& GetGeoParticlesToggle() { return geo_particles_toggle; }

	/*
	* Adds a local light to the list of local lights for rendering
	* Returns: LocalLight* - Pointer to the local light that was added.
	*/
	LocalLight* AddLocalLight(Vec3 position, Vec3 color, float radius);
	LocalLight* AddLocalLight();

	void SetLocalLightPosition(LocalLight* p_local_light, Vec3 pos);
	void SetLocalLightColor(LocalLight* p_local_light, Vec3 color);
	void SetLocalLightRadius(LocalLight* p_local_light, Float32 radius);
	void SetLocalLightToggle(ConeLocalLight* p_local_light, bool _toggle);

	/*
	* Adds a cone local light to the list of cone local lights for rendering
	* Returns: ConeLocalLight* - Pointer to the local light that was added.
	*/
	ConeLocalLight* AddConeLocalLight(Vec3 position, Quat orientation, 
									  Vec3 color, float radius, float length);
	ConeLocalLight* AddConeLocalLight();

	void SetConeLocalLightPosition(ConeLocalLight* p_local_light, Vec3 pos);
	void SetConeLocalLightOrientation(ConeLocalLight* p_local_light, Quat orient);
	void SetConeLocalLightColor(ConeLocalLight* p_local_light, Vec3 color);
	void SetConeLocalLightRadius(ConeLocalLight* p_local_light, Float32 radius);
	void SetConeLocalLightLength(ConeLocalLight* p_local_light, Float32 length);
	void SetConeLocalLightToggle(ConeLocalLight* p_local_light, bool _toggle);

	/*
	* Remove all local lights in the scene
	* Including cone local lights
	* Returns: void
	*/
	void RemoveAllLocalLights();

	/*
	* Removes the specified local light
	*/
	void RemoveLocalLight(LocalLight* p_local_light);

	/*
	* Removes the specified cone local light
	*/
	void RemoveConeLocalLight(ConeLocalLight* p_cone_local_light);

	/*Dynamically set the data for a GL vertex buffer and send to the GPU
	* Returns: void
	*/
	void SetDynamicBufferData(GLuint vao_id, GLuint vertex_buffer_id,
		float const* data, size_t data_size);

	/*
	* Set the position of the primary light used for rendering
	* Args:
	* Vec3 - position of the light
	*/
	void SetPrimaryLightPosition(const Vec3& _pos);

	/*
	* Getter for the direction of the shadow
	* Returns:
	*	Vec3 - The current Shadow look target
	*/
	Vec3 GetShadowLookTarget();

	/*
	* Setter for the direction of the shadow
	* Args:
	* Vec3 - The target position to look at for the shadow
	*/
	void SetShadowLookTarget(const Vec3& target);

	/*
	*	Toggle FullScreen	
	*/
	void ToggleFullScreen();
};

//Declared as an extern variable so it can be accessed throughout the project
extern GraphicsManager* p_graphics_manager;



// Inline definitions
inline void GraphicsManager::ToggleDebugDrawing() noexcept {
	if (debug_drawing_enabled) {
		dbg_draw = !dbg_draw;
		SIK_INFO("Debug drawing = {}", dbg_draw);
	}
}

inline void GraphicsManager::EnableDebugDrawing() noexcept {
	debug_drawing_enabled = true;
}