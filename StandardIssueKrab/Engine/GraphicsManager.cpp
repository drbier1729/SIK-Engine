/******************************************************************************/
/* !
/* File GraphicsManager.cpp
/* Author Andrew Rudasics
/* Email: andrew.rudasics@digipen.edu
/* Date 9/11/2022
/* Global manager to handle rendering of graphics objects.
/* DigiPen Institute of Technology © 2022
/******************************************************************************/

#include "stdafx.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "FrameTimer.h"
#include "GUIObject.h"
#include "GUIRenderer.h"
#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "SkinnedMesh.h"
#include "PhysicsManager.h"
#include "ParticleSystem.h"
#include "GameObjectManager.h"
#include "RenderCam.h"
#include "GraphicsManager.h"

#ifdef _DEBUG
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in GraphicsManager.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }
#else
#define CHECKERROR void(0);
#endif

// Static (class global) variable
Bool GraphicsManager::debug_drawing_enabled = false;

inline constexpr Uint8 MAX_LOCAL_LIGHTS = 64;
inline constexpr Uint8 MAX_CONE_LOCAL_LIGHTS = 64;

// Returns a perspective projection matrix
glm::mat4 Perspective(const float rx, const float ry,
    const float front, const float back)
{
    glm::mat4 P;
    P[0].x = 1 / rx;
    P[1].y = 1 / ry;
    P[2].z = (-1 * (back + front)) / (back - front);
    P[2].w = -1;
    P[3].z = (-2 * (back * front)) / (back - front);
    P[3].w = 0;
    return P;
}

// TODO (Dylan) : remove
#include "BoxGeometry.h"
#include "Line.h"
GraphicsManager::GraphicsManager()
    : window_width(1280), window_height(720),
    skyOn(false), bgOn(true), bgTexOn(false), color1(Vec3(1)), color2(Vec3(0)),
    normalsOn(false), msaa_toggle(false), wireframe_toggle(false), vsync_toggle(true),
    geo_particles_toggle(false), shadow_look_target(0), debug_local_lights(false),
    gamma(2.2f), exposure(3.0f),
    bloom_enabled(true), bloom_threshold(0.2f), bloom_factor(1.0f),
    bloom_mip_level(0), bloom_downsample_passes(8),
    deferred_shading_enabled(true) {
    
    DirectionalLight sun;
    sun.color = Vec3(1);
    sun.position = Vec3(20, 20, 20);
    sun.direction = Vec3(-1, -1, -1);
    sun.intensity = 2.0f;
    m_lights.push_back(sun);

    local_lights.reserve(MAX_LOCAL_LIGHTS);
    cone_local_lights.reserve(MAX_CONE_LOCAL_LIGHTS);
}

/*
* Clears an FBO. If no FBO is specified then clears the render buffer
* Returns: void
*/
void GraphicsManager::ClearBuffer(Vec4 clear_color, FBO* _buffer_to_clear) {
    if (_buffer_to_clear != nullptr)
        _buffer_to_clear->Bind();
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_buffer_to_clear != nullptr)
        _buffer_to_clear->Unbind();

    CHECKERROR
}

void GraphicsManager::Update(Float32 dt) {
}


void GraphicsManager::EnableFrontFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}

void GraphicsManager::EnableBackFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void GraphicsManager::DisableFaceCulling() {
    glDisable(GL_CULL_FACE);
}

void GraphicsManager::EnableDepthTest() {
    glEnable(GL_DEPTH_TEST);
}

void GraphicsManager::DisableDepthTest() {
    glDisable(GL_DEPTH_TEST);
}

void GraphicsManager::EnableMultiSampling() {
    glEnable(GL_MULTISAMPLE);
}

//Create the FBOs required by the graphics manager
void GraphicsManager::CreateFBOs() {
    shadow_fbo = std::make_unique<FBO>(4096, 4096);
    // MSAA	
    msaa_fbo = std::make_unique<MSAA>(16, window_width, window_height);

    gbuffer_fbo = std::make_unique<FBO>(window_width, window_height, 5);

    CreateBloomFBO();
}

//Delete all the created FBOs
void GraphicsManager::DeleteFBOs() {
    post_process_fbo.reset();
    gbuffer_fbo.reset();
    shadow_fbo.reset();
    msaa_fbo.reset();
}

GraphicsManager::~GraphicsManager() {
    DeleteVAO(full_screen_quad);
    DeleteFBOs();
    DestroyWindow();
}

/*Create a quad that fills the entire screen.
* Used to render the gbuffer
* Returns: GLuint - vao_id of the quad
*/
VertexArrayObject GraphicsManager::GenerateFullScreenQuad() {
	float vertices[12] = {
		-1,  1, 0,
		-1, -1, 0,
		 1, -1, 0,
		 1,  1, 0
	};

	//Create a VAO and put the ID in vao_id
	GLuint vao_id;
	glGenVertexArrays(1, &vao_id);
	//Use the same VAO for all the following operations
	glBindVertexArray(vao_id);

	//Create a continguous buffer for all the vertices/points
	GLuint point_buffer;
	glGenBuffers(1, &point_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;
	//IBO data
	GLuint indeces[6] = { 0, 1, 2, 0, 2, 3 };
	//Create IBO
	GLuint indeces_buffer;
	glGenBuffers(1, &indeces_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indeces,
		GL_STATIC_DRAW);
	CHECKERROR;
    //If you get an OPEN_GL error here then your laptop graphics settings probably aren't enabled. 

	glBindVertexArray(0);

	return VertexArrayObject{
		.vao_id = vao_id,
		.vertex_buffer_id = point_buffer,
		.index_buffer_id = indeces_buffer};
}

/*
* Performs a shadow pass. 
* Draws the depth of all objects into a depth buffer from the POV of the light.
* Returns - void
*/
void GraphicsManager::ShadowPass() {

    shadow_fbo->Bind();

    EnableFrontFaceCulling();


    EnableDepthTest();

    ClearBuffer(Vec4(0.0f));

    BindShaderProgram(*shadow_program);

    Mat4 model_transform;
    
    for (DirectionalLight& d : m_lights) {

        float light_dist = glm::length(d.position - shadow_look_target);
        Mat4 shadow_proj = GetShadowProjMatrix(d.position);

        float min_depth = light_dist - 25;
        float max_depth = light_dist + 25;
        SetUniform(*shadow_program, min_depth, "min_depth");
        SetUniform(*shadow_program, max_depth, "max_depth");

        Mat4 shadow_view = GetShadowViewMatrix(d.position);
        // Iterate through all mesh_renderer components
        for (auto m_rend = m_renderers.all(); not m_rend.is_empty(); m_rend.pop_front()) {

            MeshRenderer& mr = m_rend.front();
            if (mr.owner == nullptr || !mr.is_valid || not mr.enabled) { continue; }

            model_transform = Mat4(0);
            Transform* tr = mr.owner->HasComponent<Transform>();
            if (tr != nullptr) {
                model_transform = tr->ToMat4();
            }

            SetUniform(*shadow_program, shadow_proj, "shadow_proj");
            SetUniform(*shadow_program, shadow_view, "shadow_view");
            SetUniform(*shadow_program, model_transform, "model");

            if (mr.mesh != nullptr) {
                Mesh* m = mr.mesh;
                m->Use();
                m->Draw();
            }
        }
    }

    UnbindShaderProgram();

    DisableFaceCulling();

    shadow_fbo->Unbind();
    CHECKERROR;
}

/*
* Performs the lighting pass.
* Returns : void
*/
void GraphicsManager::LightingPass() {

    EnableDepthTest();

    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();
    Vec3 camera_pos = active_camera->GetPosition();

    Mat4 model_transform;
    Mat4 norm_inverse;
    Mat4 shadow_matrix;

    Mat4 shadow_proj = glm::perspectiveFov(glm::radians(45.0f),
        (float)shadow_fbo->GetWidth(), (float)shadow_fbo->GetHeight(),
        0.1f, 1000.0f);

    // Forward pass per directional light, TODO: Replace with deferred pass

    for (DirectionalLight const& d : m_lights) {

        Vec3 light_dir = glm::normalize(d.position);
        Vec3 light_color = d.color * d.intensity;
        shadow_matrix = CalculateShadowMatrix(d.position);

        float light_dist = glm::length(d.position);
        float min_depth = light_dist - 25;
        float max_depth = light_dist + 25;

        // Iterate through all mesh_renderer components
        for (auto m_rend = m_renderers.all(); not m_rend.is_empty(); m_rend.pop_front()) {

            MeshRenderer& mr = m_rend.front();
            if (mr.owner == nullptr) { continue; }

            model_transform = Mat4(0);
            Transform* tr = mr.owner->HasComponent<Transform>();
            if (tr != nullptr) {
                model_transform = tr->ToMat4();
            }

            norm_inverse = glm::transpose(glm::inverse(model_transform));

            GLuint sp = *mr.material->shader;

            if (sp != GetBoundShader())
                BindShaderProgram(*mr.material->shader);
            mr.Use();
            
            SetUniform(sp, min_depth, "min_depth");
            SetUniform(sp, max_depth, "max_depth");

            SetUniform(sp, model_transform, "model");
            SetUniform(sp, view, "view");
            SetUniform(sp, proj, "proj");

            SetUniform(sp, 32.0f, "material.shininess");
            SetUniform(sp, light_dir, "light.direction");
            SetUniform(sp, Vec3{ 0.05f, 0.05f, 0.05f }, "light.ambient");
            SetUniform(sp, light_color, "light.diffuse");
            SetUniform(sp, Vec3{ 0.5f, 0.5f, 0.5f }, "light.specular");

            mr.Draw();

            shadow_fbo->UnbindTexture(0, 5);
        }
    }
    UnbindShaderProgram();
    CHECKERROR;
}

/*
* Performs the GBuffer pass.
* Returns : void
*/
void GraphicsManager::GBufferPass() {

    gbuffer_fbo->Bind();

    //Drawskydome for deferred pass
    if (skyOn) DrawSkyDome(true);
    if (bgOn) DrawBackgound(true);

    BindShaderProgram(*gbuffer_program);

    GLenum bufs[5] = { GL_COLOR_ATTACHMENT0_EXT , GL_COLOR_ATTACHMENT1_EXT , GL_COLOR_ATTACHMENT2_EXT , GL_COLOR_ATTACHMENT3_EXT, GL_COLOR_ATTACHMENT4_EXT };
    glDrawBuffers(5, bufs);

    ClearBuffer(Vec4(0.5f, 0.5f, 0.5f, 1.0f));

    Mat4 model_transform;
    Mat4 norm_inverse;

    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();

    //Set the draw_id to 0 for all other Objects;
    SetUniform(bound_shader_program, 0, "draw_id");
    
    // Set view and projection matrices
    SetUniform(bound_shader_program, view, "view");
    SetUniform(bound_shader_program, proj, "proj");

    // Optimization to call Mesh::Use on all cubes at once
    Array<MeshRenderer*, 1024> cube_renderers{};
    Uint32 num_cubes = 0;
    Mesh* cube_ptr = Mesh::CubePtr();

    for (DirectionalLight const& d : m_lights) {

        // Iterate through all mesh_renderer components
        for (auto m_rend = m_renderers.all(); not m_rend.is_empty(); m_rend.pop_front()) {

            MeshRenderer& mr = m_rend.front();
            if (mr.owner == nullptr) { continue; }
            if (mr.enabled == false) { continue; }

            if (mr.mesh == cube_ptr && mr.enabled) {
                if (num_cubes < cube_renderers.size()) {
                    cube_renderers[num_cubes++] = std::addressof(mr);
                }
                continue;
            }

            model_transform = Mat4(0);
            Transform* tr = mr.owner->HasComponent<Transform>();
            if (tr != nullptr) {
                model_transform = tr->ToMat4();
            }

            norm_inverse = glm::transpose(glm::inverse(model_transform));

            mr.Use();
            SetUniform(bound_shader_program, model_transform, "model");
            SetUniform(bound_shader_program, norm_inverse, "norm_inverse");
            mr.Draw();
        }

        // Draw all cubes in a batch
        if (num_cubes > 0) {

            // First sort by material
            std::sort(cube_renderers.begin(), cube_renderers.begin() + num_cubes,
                [](MeshRenderer* mrA, MeshRenderer* mrB) {
                    return mrA->material < mrB->material;
                }
            );
            Material* cur_material = cube_renderers[0]->material;

            cube_ptr->Use();
            cur_material->Use();
            for (Uint32 i = 0; i < num_cubes; ++i) {

                MeshRenderer& mr = *cube_renderers[i];

                Transform* tr = mr.owner->HasComponent<Transform>();
                model_transform = tr->ToMat4();
                norm_inverse = glm::transpose(glm::inverse(model_transform));

                // Change material if we have to
                if (cur_material != mr.material) {
                    cur_material->Unuse();
                    cur_material = mr.material;
                    cur_material->Use();
                }

                SetUniform(bound_shader_program, model_transform, "model");
                SetUniform(bound_shader_program, norm_inverse, "norm_inverse");
                cube_ptr->Draw();
            }
            cur_material->Unuse();
            cube_ptr->Unuse();
        }
    }

    GLenum d_bufs[1] = { GL_COLOR_ATTACHMENT0_EXT };
    glDrawBuffers(1, d_bufs);

    gbuffer_fbo->Unbind();
    UnbindShaderProgram();
    CHECKERROR;
}

/*
* Performs the Deferred Lighting pass.
* Returns : void
*/
void GraphicsManager::DeferredLightingPass() {
    BindShaderProgram(*deferred_lighting_program);

    gbuffer_fbo->BindTexture(*deferred_lighting_program, "g_buffer_world_pos", 0);
    gbuffer_fbo->BindTexture(*deferred_lighting_program, "g_buffer_world_norm", 1);
    gbuffer_fbo->BindTexture(*deferred_lighting_program, "g_buffer_diffuse_color", 2);
    gbuffer_fbo->BindTexture(*deferred_lighting_program, "g_buffer_specular_color", 3);
    gbuffer_fbo->BindTexture(*deferred_lighting_program, "g_buffer_emission_color", 4);
    shadow_fbo->BindTexture(*deferred_lighting_program, "shadow_map", 0, 6);

    SetUniform(*deferred_lighting_program, gbuffer_fbo->GetWidth(), "buffer_width");
    SetUniform(*deferred_lighting_program, gbuffer_fbo->GetHeight(), "buffer_height");

    Vec3 camera_pos = active_camera->GetPosition();
    SetUniform(*deferred_lighting_program, camera_pos, "view_pos");

    Mat4 shadow_mat;

    SetUniform(*deferred_lighting_program, gamma, "gamma");

    for (DirectionalLight const& d : m_lights) {

        float light_dist = glm::length(d.position);
        float min_depth = light_dist - 25;
        float max_depth = light_dist + 25;
        SetUniform(*deferred_lighting_program, min_depth, "min_depth");
        SetUniform(*deferred_lighting_program, max_depth, "max_depth");

        shadow_mat = CalculateShadowMatrix(d.position);
        Vec3 light_dir = glm::normalize(d.position);
        Vec3 light_color = d.color * d.intensity;
        SetUniform(*deferred_lighting_program, light_dir, "light_dir");
        SetUniform(*deferred_lighting_program, light_color, "light_color");
        SetUniform(*deferred_lighting_program, shadow_mat, "shadow_mat");
        
        DrawFullScreenQuad();
    }

    gbuffer_fbo->UnbindTexture(*deferred_lighting_program, 1);
    gbuffer_fbo->UnbindTexture(*deferred_lighting_program, 2);
    gbuffer_fbo->UnbindTexture(*deferred_lighting_program, 3);
    gbuffer_fbo->UnbindTexture(*deferred_lighting_program, 4);
    shadow_fbo->UnbindTexture(*deferred_lighting_program, 5);
    shadow_fbo->UnbindTexture(*deferred_lighting_program, 6);

    UnbindShaderProgram();
}

/*
* Performs the local lights pass
* Returns : void
*/
void GraphicsManager::LocalLightsPass() {
    DisableDepthTest();
    EnableAdditiveBlend();
    EnableFrontFaceCulling();

    BindShaderProgram(*local_lights_program);
    CHECKERROR;

    gbuffer_fbo->BindTexture(bound_shader_program, "g_buffer_world_pos", 0);
    gbuffer_fbo->BindTexture(bound_shader_program, "g_buffer_world_norm", 1);
    gbuffer_fbo->BindTexture(bound_shader_program, "g_buffer_diffuse_color", 2);
    gbuffer_fbo->BindTexture(bound_shader_program, "g_buffer_specular_color", 3);
    CHECKERROR;

    SetUniform(bound_shader_program, gbuffer_fbo->GetWidth(), "buffer_width");
    SetUniform(bound_shader_program, gbuffer_fbo->GetHeight(), "buffer_height");
    CHECKERROR;

    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();
    SetUniform(bound_shader_program, view, "view");
    SetUniform(bound_shader_program, proj, "proj");
    CHECKERROR;

    SetUniform(bound_shader_program, gamma, "gamma");
    SetUniform(bound_shader_program, debug_local_lights, "debug_local_lights");

    CHECKERROR;
    DrawLocalLights();
    CHECKERROR;

    DisableBlend();
    DisableFaceCulling();
    EnableDepthTest();

    gbuffer_fbo->UnbindTexture(bound_shader_program, 0);
    gbuffer_fbo->UnbindTexture(bound_shader_program, 1);
    gbuffer_fbo->UnbindTexture(bound_shader_program, 2);
    gbuffer_fbo->UnbindTexture(bound_shader_program, 3);
    gbuffer_fbo->UnbindTexture(bound_shader_program, 4);

    UnbindShaderProgram();
    CHECKERROR;
}

/*
* Iterates through the list of local lights and renders them 
* Returns: void
*/
void GraphicsManager::DrawLocalLights() {
    //To fix a bug due to improper un-use from a previous pass
    //I didn't have enough time to find the root cause for a better fix.
    // sry :( 
    (Mesh::SpherePtr())->Unuse();

    //Draw the sphere local lights
    (Mesh::SpherePtr())->Use();
    for (auto& local_light : local_lights) {
        if (not local_light.enabled)
            continue;

        SetUniform(bound_shader_program, local_light.color, "local_light_color");
        SetUniform(bound_shader_program, local_light.position, "local_light_pos");
        SetUniform(bound_shader_program, local_light.radius, "local_light_radius");
        SetUniform(bound_shader_program, local_light.ease_out, "ease_out");
        
        Mat4 model = glm::translate(Mat4(1), local_light.position) * glm::scale(Mat4(1), Vec3(local_light.radius));
        SetUniform(bound_shader_program, model, "model");

        (Mesh::SpherePtr())->Draw();
        CHECKERROR;
    }
    (Mesh::SpherePtr())->Unuse();

    //Draw the cone local lights
    (Mesh::ConePtr())->Use();
    for (auto& local_light : cone_local_lights) {
        if (not local_light.enabled)
            continue;

        Mat4 scale_mat = glm::scale(Mat4(1),
            Vec3(local_light.radius * 2, //X and Y coords for the base of the cylinder
                local_light.radius * 2, //These represent the radius for the base
                local_light.length)); //Model formed with Z-Up so the length corresponds to the length from base to tip (penis joke?)
        
        Mat4 model = glm::translate(Mat4(1), local_light.position) * glm::toMat4(local_light.orientation) * scale_mat;
        SetUniform(bound_shader_program, model, "model");

        /*
        * Have to represent the cone as a sub volume within a sphere for the fragment shader.
        * The center of the sphere has to be the tip of the cone i.e. local_light_pos
        * The radius of the sphere has to be the length of the cone i.e. local_light_radius
        */
        Vec4 local_light_pos = model * Vec4(0, 0, local_light.length/2, 1);
        SetUniform(bound_shader_program, local_light.color, "local_light_color");
        SetUniform(bound_shader_program, Vec3(local_light_pos), "local_light_pos");
        SetUniform(bound_shader_program, local_light.length*2.0f, "local_light_radius"); 
        SetUniform(bound_shader_program, local_light.ease_out, "ease_out");

        (Mesh::ConePtr())->Draw();
        CHECKERROR;
    }
    (Mesh::ConePtr())->Unuse();
}

void GraphicsManager::DisableMultiSampling() {
    glDisable(GL_MULTISAMPLE);
}

void GraphicsManager::EnableAdditiveBlend() {
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
}

void GraphicsManager::EnableAlphaBlend() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void GraphicsManager::DisableBlend() {
    glDisable(GL_BLEND);
}

/*
* Makes it so that the following draw calls go to the render buffer
*/
void GraphicsManager::EnableRenderBuffer() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glViewport(0, 0, window_width, window_height);
}

/*
* Renders objects in scene to screen
*/
void GraphicsManager::RenderScene() {

    CleanupDestroyedRenderers();

    // *************** Begin GBuffer Pass *************************

    GBufferPass();


    // *************** End GBuffer Pass ***************************

    
    // *************** Begin Shadow Pass *************************

    ShadowPass();


    // *************** End Shadow Pass ***************************
    
    //Check if any of the FBOs need to be drawn
    if (draw_buffer != DrawBuffer::DISABLE) {
        switch (draw_buffer) {
        case DrawBuffer::WORLD_POS:
            //Draw WorldPos from Gbuffer
            DrawFBO(gbuffer_fbo.get(), 0);
            return;
            break;
        case DrawBuffer::NORMALS:
            //Draw Normals from Gbuffer
            DrawFBO(gbuffer_fbo.get(), 1);
            return;
            break;
        case DrawBuffer::DIFFUSE:
            //Draw Colors from Gbuffer
            DrawFBO(gbuffer_fbo.get(), 2);
            return;
            break;
        case DrawBuffer::SPECULAR:
            //Draw Specular colors from Gbuffer
            DrawFBO(gbuffer_fbo.get(), 3);
            return;
            break;
        case DrawBuffer::EMISSION:
            //Draw Specular colors from Gbuffer
            DrawFBO(gbuffer_fbo.get(), 4);
            return;
            break;
        case DrawBuffer::SHADOW:
            //Draw depth from shadow map
            DrawFBO(shadow_fbo.get());
            return;
            break;
        }
    }

    // *************** Begin Deferred Lighting Pass ***********************
    if (msaa_toggle) {
        msaa_fbo->Bind();
        EnableMultiSampling();
    }
    else {
        post_process_fbo->Bind();
    }

    ClearBuffer(Vec4(0.2f, 0.3f, 0.3f, 1.0f));

    if (deferred_shading_enabled) {
        DeferredLightingPass();
    }

    // *************** End Deferred Lighting Pass **********************
    
    // *************** Begin Lighting Pass ***********************
    else {
        EnableBackFaceCulling();

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "4");
        glPolygonMode(GL_FRONT_AND_BACK, wireframe_toggle ? GL_LINE : GL_FILL);

        // draw sky dome / background
        if (skyOn) DrawSkyDome();
        if (bgOn)  DrawBackgound();

        LightingPass();
    }
    
	// *************** End Lighting Pass **********************
    
    // *************** Begin Local Lights Pass **********************
    if (deferred_shading_enabled)
        LocalLightsPass();

    // *************** End Local Lights Pass **********************

    // *************** Particles rendering **********************
    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();
    DisableDepthTest();
    EnableAlphaBlend();
    p_particle_system->Draw(geo_particles_toggle ? *geo_particle_shader : *particle_shader, proj * view, active_camera->GetCameraRight(), active_camera->GetCameraUp());
    DisableBlend();
    // *************** End Particles rendering **********************

    if (msaa_toggle) {
        msaa_fbo->Draw(post_process_fbo.get());
        msaa_fbo->Unbind();
        DisableMultiSampling();
    }

    // *************** Bloom calculation **********************
    if (bloom_enabled)
        BloomPass();
    // *************** End Bloom calculation **********************

    //Start drawing to the screen buffer now.
    EnableRenderBuffer();
    PostProcessPass();
    
    // *************** GUI rendering **********************
    GUIRender();
    // *************** End GUI rendering **********************

    // Debug drawing code
    if (debug_drawing_enabled && dbg_draw) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        p_physics_manager->DebugDraw(*wireframe_shader, active_camera, 0.0f, *phys_box);
        p_physics_manager->DebugDraw_Forces(*wireframe_shader, active_camera, 0.0f, *arrow);
        DrawNormals();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void GraphicsManager::GUIRender() {
	GLuint* gui_shader = p_resource_manager->GetShaderProgram("gui"_sid);
	BindShaderProgram(*gui_shader);
	BindAttrib(*gui_shader, 0, "in_position");
	BindAttrib(*gui_shader, 1, "in_TexCoords");

	SetDepthTestOff();
    EnableAlphaBlend();
	SetUniform(*gui_shader, ortho_proj, "ortho_projection");
    for (auto r = gui_renderer_list.all(); not r.is_empty();  r.pop_front()) {
        auto& gr = r.front();
        gr.Draw();
	}
	SetDepthTestOn();
    DisableBlend();
    CHECKERROR;
}

/*
* Swap the window buffer with the draw buffer in double buffered setups
* Returns: void
*/
void GraphicsManager::SwapBuffers() {
    SDL_GL_SwapWindow(p_window);
}


/*
* Initializes the window that we render to
* Sets OpenGL version, Creates SDLwindow and openGL context
* Sets Vsync
* Returns: bool - True on successfull p_window creation
*/
bool GraphicsManager::InitWindow() {
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SIK_ERROR("SDL could not initialize! SDL_Error: \"{}\"", SDL_GetError());
		return false;
	}
	
	// Set OpenGL version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //Create p_window
    p_window = SDL_CreateWindow("Drifty Thrifty Bang Bang", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (p_window == NULL) {

        SIK_ERROR("Window could not be created! SDL_Error: \"{}\"", SDL_GetError());
        return false;
    }

    // Create OpenGL context
    gl_context = SDL_GL_CreateContext(p_window);
    if (gl_context == NULL) {
        SIK_ERROR("OpenGL context could not be created! SDL_Error: \"{}\"", SDL_GetError());
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
        /*
        * This reinterpret_cast is necessary because glewGetErrorString returns
        * const GLubyte* which can only be static_cast'd to const unsigned char*
        * BUT spdlog can't handle unsigned char...
        */
        auto err_str = reinterpret_cast<const char*>(glewGetErrorString(glew_error));
        SIK_ERROR("Error initializing GLEW! glew_Error: \"{}\"", err_str);
        return false;
    }

    // Use Vsync
    //if (SDL_GL_SetSwapInterval(1) < 0) {
    //    SIK_ERROR("Warning: Unable to set VSync! SDL Error: \"{}\"", SDL_GetError());
    //    return false;
    //}

    return true;
}

/*
* Destory the SDL p_window and delete GLContext
* Sets OpenGL version, Creates SDLwindow and openGL context
* Sets Vsync
* Returns: void
*/
void GraphicsManager::DestroyWindow() {
    SDL_DestroyWindow(p_window);
    SDL_GL_DeleteContext(gl_context);
}

void GraphicsManager::DeleteVAO(VertexArrayObject& vao) {
    //Bind the vao before deleting the buffers that are part of the vao
    glBindVertexArray(vao.vao_id);
    glDeleteBuffers(1, &vao.vertex_buffer_id);
    glDeleteBuffers(1, &vao.index_buffer_id);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao.vao_id);
    CHECKERROR;
}


Mat4 GraphicsManager::GetShadowProjMatrix(const Vec3& light_pos) {
    Mat4 shadow_proj = glm::perspectiveFov(glm::radians(60.0f),
        (float)shadow_fbo->GetWidth(), (float)shadow_fbo->GetHeight(),
        0.1f, 1000.0f);

    return shadow_proj;
}

Mat4 GraphicsManager::GetShadowViewMatrix(const Vec3 light_pos) {
    return glm::lookAt(light_pos, shadow_look_target, Vec3(0, 1, 0));
}

/*
* Gets the shadow matrix used for shadow calculation
* Returns: Mat4 - the shadow matrix transformation
*/
Mat4 GraphicsManager::CalculateShadowMatrix(const Vec3& light_pos) {
    Mat4 shadow_proj = GetShadowProjMatrix(light_pos);
    Mat4 shadow_view = GetShadowViewMatrix(light_pos);
    Mat4 shadow_matrix = 
        glm::translate(Mat4(1), Vec3(0.5f)) *
        glm::scale(Mat4(1), Vec3(0.5f)) *
        shadow_proj * shadow_view;
    return shadow_matrix;
}

void GraphicsManager::CreateBloomFBO() {
    post_process_fbo = std::make_unique<FBO>(window_width, window_height, 3);
    Int16 downsample_width, downsample_height;

    for (unsigned int i = 1; i <= 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, post_process_fbo->textureID[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, bloom_downsample_passes);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        downsample_width = post_process_fbo->GetWidth() / 2;
        downsample_height = post_process_fbo->GetHeight() / 2;
        glBindTexture(GL_TEXTURE_2D, post_process_fbo->textureID[i]);
        for (int mip_level = 0; mip_level < bloom_downsample_passes; ++mip_level) {
            glTexImage2D(GL_TEXTURE_2D, mip_level + 1, (int)GL_RGBA32F,
                downsample_width, downsample_height, 0, GL_RGBA, GL_FLOAT, NULL);
            CHECKERROR;
            downsample_width /= 2;
            downsample_height /= 2;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

/*
* Returns the GLContext initialized during p_window creation
* Returns: SDL_GLContext
*/
SDL_GLContext GraphicsManager::GetGLContext() {
    return gl_context;
}

/*
* Returns the pointer to SDL p_window created during p_window creation
* Returns: SDL_Window*
*/
SDL_Window* GraphicsManager::GetPWindow() {
    return p_window;
}

/*
* Returns the active render cam
*/
RenderCam* GraphicsManager::GetPActiveCam() {
    return active_camera;
}

//Sets the active render cam
void GraphicsManager::SetPActiveCam(RenderCam* _p_cam) {
    active_camera = _p_cam;
}

void GraphicsManager::BindShaderProgram(GLuint shader_program) {
    if (bound_shader_program == shader_program)
        return;

    glUseProgram(shader_program);
    bound_shader_program = shader_program;
    CHECKERROR
}

GLuint GraphicsManager::GetBoundShader() const {
    return bound_shader_program;
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Int32& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniform1i(loc, val);
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Uint32& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniform1ui(loc, val);
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Float32& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniform1f(loc, val);
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Vec3& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniform3fv(loc, 1, &val[0]);
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Vec4& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniform4fv(loc, 1, &val[0]);
}

void GraphicsManager::SetUniform(
    GLuint shader_program, const Mat4& val, const char* var_name) const {
    int loc = glGetUniformLocation(shader_program, var_name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, &val[0][0]);
}

/*Function to bind an attribute locations
* for the given variable
* Returns: void
*/
void GraphicsManager::BindAttrib(GLuint shader_program, 
                                 GLuint attrib, const std::string& var_name) {
    glBindAttribLocation(shader_program, attrib, var_name.c_str());
    CHECKERROR;
}

/*Function to bind an output attribute locations
* for the given variable
* Returns: void
*/
void GraphicsManager::BindOutputAttrib(GLuint shader_program, 
                                       GLuint attrib, const std::string& var_name) {
    glBindFragDataLocation(shader_program, attrib, var_name.c_str());
    CHECKERROR;
}

/*
* Calls Capacity() on the underlying memory resource
* Returns: size_t - the space left in bytes
*/
SizeT GraphicsManager::BufferSpaceAvailable() const {
    SIK_ASSERT(false, "Deprecated");
    return gui_renderer_list.capacity() - gui_renderer_list.size();
}

/*
* Calls Size() on the underlying memory resource
* Returns: size_t - the space left in bytes
*/
SizeT GraphicsManager::BufferSpaceUsed() const {
    SIK_ASSERT(false, "Deprecated");
    return gui_renderer_list.size();
}

/*
* Performs the render pass for adding bloom
* 
*/
void GraphicsManager::BloomPass() {
    //Bloom threshold check and copy
    BindShaderProgram(*bloom_program);
    post_process_fbo->BindTexture(*bloom_program, "input_buffer", 0, 0);
    post_process_fbo->BindImageTexture(
        *bloom_program, "dst", 1, 1, 0);

    SetUniform(*bloom_program, window_width, "width");
    SetUniform(*bloom_program, window_height, "height");
    SetUniform(*bloom_program, bloom_threshold, "bloom_threshold");
    glDispatchCompute(window_width, window_height, 1);
    CHECKERROR;
    UnbindShaderProgram();

    ////////////////////////////////////////////////////////////////////////////////
    // Bloom pass downsampling
    ////////////////////////////////////////////////////////////////////////////////
    int start_width = window_width;
    int start_height = window_height;
    int downsampling_width = window_width / 2;
    int downsampling_height = window_height / 2;
    std::vector<int> width_list;
    std::vector<int> height_list;
    width_list.push_back(start_width);
    height_list.push_back(start_height);

    BindShaderProgram(*downsample_program);
    post_process_fbo->BindTexture(*downsample_program, "input_buffer", 1, 0);
    for (Int8 mip_level = 0; mip_level < bloom_downsample_passes; ++mip_level) {
        SetUniform(*downsample_program, static_cast<Float32>(mip_level), "mip_level");
        post_process_fbo->BindImageTexture(
            *downsample_program, "dst", 1, 1, mip_level + 1);

        //width and height before the downsampling is performed
        SetUniform(*downsample_program, start_width, "width");
        SetUniform(*downsample_program, start_height, "height");

        // Runs with half width and half height of the previous pass.
        glDispatchCompute(downsampling_width, downsampling_height, 1);
        CHECKERROR;

        start_width = downsampling_width;
        start_height = downsampling_height;
        downsampling_width /= 2;
        downsampling_height /= 2;

        width_list.push_back(start_width);
        height_list.push_back(start_height);
    }
    UnbindShaderProgram();

    ////////////////////////////////////////////////////////////////////////////////
    // End of Bloom pass downsample
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // Bloom pass upsample + additive blending
    ////////////////////////////////////////////////////////////////////////////////

    BindShaderProgram(*upsample_program);
    post_process_fbo->BindTexture(*upsample_program, "input_buffer", 1, 0);

    for (Int8 mip_level = bloom_downsample_passes; mip_level > 0; --mip_level) {
        SetUniform(*upsample_program, static_cast<Float32>(mip_level), "mip_level");

        post_process_fbo->BindImageTexture(
            *upsample_program, "dst", 2, 1, mip_level - 1);

        //width and height before the upsampling is performed
        SetUniform(*upsample_program, width_list[mip_level], "width");
        SetUniform(*upsample_program, height_list[mip_level], "height");

        // Runs with double width and double height of the previous pass.
        glDispatchCompute(width_list[mip_level - 1], height_list[mip_level - 1], 1);
        CHECKERROR;
    }
    UnbindShaderProgram();
    ////////////////////////////////////////////////////////////////////////////////
    // End of Bloom pass upsample + additive blending
    ////////////////////////////////////////////////////////////////////////////////
}

void GraphicsManager::PostProcessPass() {
    ClearBuffer(Vec4(0.2f, 0.3f, 0.3f, 1.0f));

    // Choose the post processing shader
    BindShaderProgram(*post_process_program);
    CHECKERROR;

    post_process_fbo->BindTexture(bound_shader_program, "render_buffer", 0, 0);
    post_process_fbo->BindTexture(bound_shader_program, "downsample_buffer", 1, 1);
    post_process_fbo->BindTexture(bound_shader_program, "upsample_buffer", 2, 2);
    CHECKERROR;

    SetUniform(bound_shader_program, static_cast<int>(draw_buffer), "draw_buffer");
    CHECKERROR;

    SetUniform(bound_shader_program, window_width, "width");
    SetUniform(bound_shader_program, window_height, "height");
    CHECKERROR;

    SetUniform(bound_shader_program, exposure, "exposure");
    SetUniform(bound_shader_program, gamma, "gamma");
    CHECKERROR;

    SetUniform(bound_shader_program, bloom_enabled, "bloom_enabled");
    SetUniform(bound_shader_program, bloom_factor, "bloom_factor");
    SetUniform(bound_shader_program, static_cast<Float32>(bloom_mip_level), "bloom_mip_level");

    CHECKERROR;
    //Draw a full screen quad to activate every pixel shader
    DrawFullScreenQuad();

    // Turn off the shader
    UnbindShaderProgram();
}

/*
* Draw the normals for all the mesh renderers
*/
void GraphicsManager::DrawNormals() {
    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();
    Mat4 model_transform;
    Mat4 norm_inverse;
    // Iterate through all mesh_renderer components
    for (auto m_rend = m_renderers.all(); not m_rend.is_empty(); m_rend.pop_front()) {

        MeshRenderer& mr = m_rend.front();
        if (mr.owner == nullptr) { continue; }

        model_transform = Mat4(0);
        Transform* tr = mr.owner->HasComponent<Transform>();
        if (tr != nullptr) {
            model_transform = tr->ToMat4();
        }

        norm_inverse = glm::transpose(glm::inverse(model_transform));

        if (normalsOn) {
            BindShaderProgram(*normal_program);

            SetUniform(*normal_program, model_transform, "model");
            SetUniform(*normal_program, view, "view");
            SetUniform(*normal_program, proj, "proj");
            SetUniform(*normal_program, norm_inverse, "norm_inverse");

            if (mr.mesh != nullptr) {
                Mesh* m = mr.mesh;

                m->Use();
                m->Draw();;
            }

            UnbindShaderProgram();
        }
    }
}

/*
* Creates a GUIRenderer object and adds it to the list
* Returns: GUIRenderer* - pointer to the created object
*/
GUIRenderer* GraphicsManager::CreateGUIRenderer(GUIObject* gui_obj) {
    return gui_renderer_list.emplace(gui_obj);
}

/*
* Delete all GUI renderer objects
* Returns: void
*/
void GraphicsManager::DeleteAllGUIRenderers() {
    gui_renderer_list.clear();
}

/*
* Sets the orthographic projection based on the window size
* Returns: void
*/
void GraphicsManager::SetOrthoProj() {
	float right  = static_cast<float>(window_width);
	float bottom = static_cast<float>(window_height);
	ortho_proj = glm::ortho(0.0f, right, bottom, 0.0f, -1.0f, 1.0f);
}

void GraphicsManager::UnbindShaderProgram() {
    glUseProgram(0);
    bound_shader_program = 0;
}

//Intialize all the required shaders for the program
void GraphicsManager::InitShaders() {
	wireframe_shader = p_resource_manager->GetShaderProgram("debug"_sid);
	shader_program = p_resource_manager->GetShaderProgram("lit"_sid);
	particle_shader = p_resource_manager->GetShaderProgram("particles"_sid);

	// shader for sky dome
	skyShader_program = p_resource_manager->GetShaderProgram("skydome"_sid);

	// shader for background
	bgShader_program = p_resource_manager->GetShaderProgram("background"_sid);

	shadow_program = p_resource_manager->GetShaderProgram("shadow"_sid);
	fbo_program = p_resource_manager->GetShaderProgram("buffer_output"_sid);
	normal_program = p_resource_manager->GetShaderProgram("normals"_sid);
    
    //Setup Gbuffer program
    gbuffer_program = p_resource_manager->LoadShaderProgram("gbuffer");
    BindOutputAttrib(*gbuffer_program, 0, "out_world_pos");
    BindOutputAttrib(*gbuffer_program, 1, "out_world_norm");
    BindOutputAttrib(*gbuffer_program, 2, "out_diffuse_color");
    BindOutputAttrib(*gbuffer_program, 3, "out_specular_color");
    BindOutputAttrib(*gbuffer_program, 4, "out_emission_color");

    deferred_lighting_program = 
        p_resource_manager->LoadShaderProgram("deferred_lighting");
    BindOutputAttrib(*deferred_lighting_program, 0, "render_buffer");

    geo_particle_shader = p_resource_manager->GetShaderProgram("geo_particles"_sid);

    local_lights_program = 
        p_resource_manager->LoadShaderProgram("local_lights");
    
    post_process_program = p_resource_manager->LoadShaderProgram("post_process");

    downsample_program = p_resource_manager->LoadComputeShader("downsample");
    upsample_program = p_resource_manager->LoadComputeShader("upsample");
    bloom_program = p_resource_manager->LoadComputeShader("bloom");
	CHECKERROR;
}

//Set Depth test on
void GraphicsManager::SetDepthTestOn() {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

//Set Depth test off
void GraphicsManager::SetDepthTestOff() {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

//Teardown to clear the object pools
void GraphicsManager::Clear() {
    m_renderers.clear();
}

/*
* Moved constructor vao setup code here
* Returns : Void
*/
void GraphicsManager::Setup() {
    if (debug_drawing_enabled) {
        SIK_WARN("GraphicsManager scene set up with debug drawing enabled. \
                  Use ` key to toggle on/off.");
    }

	glFrontFace(GL_CCW);	
	
    SetOrthoProj();
    CreateFBOs();
    full_screen_quad = GenerateFullScreenQuad();

	// sky dome
	SkyDome = Mesh::SpherePtr();	
	Background = Mesh::PlanePtr();

	// TODO (dylan) : remove this
	phys_box = std::make_unique<PhysDebugBox>();
	arrow = std::make_unique<Arrow>();
}

MeshRenderer* GraphicsManager::CreateMeshRenderer(Material* mat, Mesh* mesh) {
	MeshRenderer mr{};
	mr.material = mat;
	mr.mesh = mesh;
	mr.is_valid = true;

    return m_renderers.insert(mr);
}

void GraphicsManager::DestroyMeshRenderer(MeshRenderer* mr) {
    mr->is_valid = false;
}

void GraphicsManager::CleanupDestroyedRenderers() {

	for (auto m_rend = m_renderers.all(); not m_rend.is_empty(); m_rend.pop_front()) {

		MeshRenderer& mr = m_rend.front();
		if (!mr.is_valid) {

			if (mr.owner != nullptr)
			{
				mr.owner = nullptr;
			}

			m_renderers.erase(&mr);
		}
	}
}

void GraphicsManager::DrawSkyDome(bool deferred) {
    Mat4 view = active_camera->GetViewMat();
    Mat4 proj = active_camera->GetProjMat();

	Texture* sky_tex = p_resource_manager->GetTexture("Tropical_Beach_8k.jpg"_sid);
	SIK_ASSERT(sky_tex, "Failed to load sky texture.");

	// switch face culling to FRONT
    EnableFrontFaceCulling();

    // set Transformation
    Mat4 model = glm::scale(Mat4(1), Vec3(500));
    model = glm::translate(model, Vec3(0));

    if (deferred) {
        BindShaderProgram(*gbuffer_program);
        SetUniform(bound_shader_program, 1, "has_color_tex");
        sky_tex->Bind(1, bound_shader_program, "base_color_tex");
        //Set the draw_id to 1 for skydomes
        SetUniform(bound_shader_program, 1, "draw_id");
        SetUniform(bound_shader_program, glm::inverse(view), "view_inverse");
    }
    else {
        BindShaderProgram(*skyShader_program);
        sky_tex->Bind(1, bound_shader_program, "texture_sampler");
        SetUniform(bound_shader_program, glm::inverse(view), "iView");
    }

	SkyDome->Use();

	// set uniforms
	SetUniform(bound_shader_program, model, "model");
	SetUniform(bound_shader_program, view, "view");
	SetUniform(bound_shader_program, proj, "proj");
	
    SkyDome->Draw();

	sky_tex->Unbind();
	SkyDome->Unuse();

    UnbindShaderProgram();

    // switch face culling to BACK
    EnableBackFaceCulling();
}

void GraphicsManager::DrawBackgound(bool deferred) {	
	Texture* bg_tex = p_resource_manager->GetTexture("goose.jpg"_sid);
	SIK_ASSERT(bg_tex, "Failed to load background texture.");

    DisableDepthTest();

    if (deferred) {
        BindShaderProgram(*gbuffer_program);
        bg_tex->Bind(1, bound_shader_program, "base_color_tex");
        SetUniform(bound_shader_program, bgTexOn, "has_color_tex");
        SetUniform(bound_shader_program, color1, "diffuse_color");
        SetUniform(bound_shader_program, color2, "diffuse_color_2");
        //Draw ID for background drawing
        SetUniform(bound_shader_program, 2, "draw_id");
    }
    else {
        BindShaderProgram(*bgShader_program);
        bg_tex->Bind(1, bound_shader_program, "texture_sampler");
        SetUniform(bound_shader_program, bgTexOn, "showTexture");
        SetUniform(bound_shader_program, color1, "color1");
        SetUniform(bound_shader_program, color2, "color2");
    }

	Background->Use();
    Background->Draw();

	bg_tex->Unbind();
	Background->Unuse();

    UnbindShaderProgram();

    EnableDepthTest();
}

void GraphicsManager::DrawFBO(FBO* fbo, Uint8 color_attachment) {
    EnableRenderBuffer();
    ClearBuffer(Vec4(0.2f, 0.3f, 0.3f, 1.0f));
    DisableDepthTest();
    BindShaderProgram(*fbo_program);

    fbo->BindTexture(*fbo_program, "fbo_texture", color_attachment, 0);
    
    SetUniform(*fbo_program, fbo->GetWidth(), "buffer_width");
    SetUniform(*fbo_program, fbo->GetHeight(), "buffer_height");
    
    DrawFullScreenQuad();

    fbo->UnbindTexture(color_attachment);
    glBindVertexArray(0);

    UnbindShaderProgram();
    EnableDepthTest();
}

/*
* Calls glDrawElements on the full screen quad vao
* Returns: void
*/
void GraphicsManager::DrawFullScreenQuad() {
    glBindVertexArray(full_screen_quad.vao_id);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    CHECKERROR;
}

void GraphicsManager::ToggleVSync() {
    if (vsync_toggle) {
        SDL_GL_SetSwapInterval(1);
    }
    else {
        SDL_GL_SetSwapInterval(0);
    }
}

/*
* Adds a local light to the list of local lights for rendering
* Returns: LocalLight*
*/
LocalLight* GraphicsManager::AddLocalLight(Vec3 _position, Vec3 _color, float _radius) {
    SIK_ASSERT(local_lights.size() < MAX_LOCAL_LIGHTS,
                "Increase the MAX_LOCAL_LIGHTS");

    local_lights.emplace_back(_position, _color, _radius);
    local_lights.back().enabled = true;
    local_lights.back().ease_out = true;
    return &(local_lights.back());
}
LocalLight* GraphicsManager::AddLocalLight() {
    SIK_ASSERT(local_lights.size() < MAX_LOCAL_LIGHTS,
                "Increase the MAX_LOCAL_LIGHTS");

    local_lights.emplace_back();
    local_lights.back().enabled = true;
    local_lights.back().ease_out = true;
    return &(local_lights.back());
}

void GraphicsManager::SetLocalLightPosition(LocalLight* p_local_light, Vec3 pos) {
    p_local_light->position = pos;
}
void GraphicsManager::SetLocalLightColor(LocalLight* p_local_light, Vec3 color) {
    p_local_light->color = color;
}

void GraphicsManager::SetLocalLightRadius(LocalLight* p_local_light, Float32 radius) {
    p_local_light->radius = radius;
}

void GraphicsManager::SetLocalLightToggle(ConeLocalLight* p_local_light, bool _toggle) {
    p_local_light->enabled = _toggle;
}

/*
* Adds a Cone local light to the list of cone local lights for rendering
* Returns: ConeLocalLight*
*/
ConeLocalLight* GraphicsManager::AddConeLocalLight(
    Vec3 _position, Quat _orientation, Vec3 _color, float _radius, float _length) {
    SIK_ASSERT(cone_local_lights.size() < MAX_CONE_LOCAL_LIGHTS,
               "Increase the MAX_CONE_LOCAL_LIGHTS");

    cone_local_lights.emplace_back(_position,  _orientation, _color, _radius, _length);
    cone_local_lights.back().enabled = true;
    cone_local_lights.back().ease_out = true;
    return &(cone_local_lights.back());
}
ConeLocalLight* GraphicsManager::AddConeLocalLight() {
    SIK_ASSERT(cone_local_lights.size() < MAX_CONE_LOCAL_LIGHTS,
                "Increase the MAX_CONE_LOCAL_LIGHTS");

    cone_local_lights.emplace_back();
    cone_local_lights.back().enabled = true;
    cone_local_lights.back().ease_out = true;
    return &(cone_local_lights.back());
}

void GraphicsManager::SetConeLocalLightPosition(
    ConeLocalLight* p_cone_local_light, Vec3 pos) {
    p_cone_local_light->position = pos;
}

void GraphicsManager::SetConeLocalLightOrientation(
    ConeLocalLight* p_cone_local_light, Quat orient) {
    p_cone_local_light->orientation = orient;
}

void GraphicsManager::SetConeLocalLightColor(
    ConeLocalLight* p_cone_local_light, Vec3 color) {
    p_cone_local_light->color = color;
}

void GraphicsManager::SetConeLocalLightRadius(
    ConeLocalLight* p_cone_local_light, Float32 radius) {
    p_cone_local_light->radius = radius;
}

void GraphicsManager::SetConeLocalLightLength(
    ConeLocalLight* p_cone_local_light, Float32 length) {
    p_cone_local_light->length = length;
}

void GraphicsManager::SetConeLocalLightToggle(ConeLocalLight* p_local_light, bool _toggle) {
    p_local_light->enabled = _toggle;
}

/*
* Remove all local lights in the scene
* Returns: void
*/
void GraphicsManager::RemoveAllLocalLights() {
    local_lights.clear();
    cone_local_lights.clear();
}

/*
* Removes a local light at the specified index
*/
void GraphicsManager::RemoveLocalLight(LocalLight* p_local_light) {
    for (auto iter = local_lights.begin(); iter != local_lights.end(); ++iter) {
        if (p_local_light == &(*iter)) {
            local_lights.erase(iter);
            return;
        }
    }
}

/*
* Removes a local light at the specified index
*/
void GraphicsManager::RemoveConeLocalLight(ConeLocalLight* p_cone_local_light) {
    for (auto iter = cone_local_lights.begin(); iter != cone_local_lights.end(); ++iter) {
        if (p_cone_local_light == &(*iter)) {
            cone_local_lights.erase(iter);
            return;
        }
    }
}

/*
* Dynamically set the data for a GL vertex buffer and send to the GPU
* Returns: void
*/
void GraphicsManager::SetDynamicBufferData(
    GLuint vao_id, GLuint vertex_buffer_id, 
    float const* data, size_t data_size) {
    glBindVertexArray(vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    CHECKERROR;
}

void GraphicsManager::SetPrimaryLightPosition(const Vec3& _pos) {
    m_lights[0].position = _pos;
}

Vec3 GraphicsManager::GetShadowLookTarget() {
    return shadow_look_target;
}

void GraphicsManager::SetShadowLookTarget(const Vec3& target) {
    shadow_look_target = target;
}

void GraphicsManager::ToggleFullScreen() {
    Uint32 flag = SDL_WINDOW_FULLSCREEN;
    bool IsFullscreen = SDL_GetWindowFlags(p_window) & flag;
    SDL_SetWindowFullscreen(p_window, IsFullscreen ? 0 : flag);
}
