/****************************************************************************** /
/* !
/* File RenderCam.cpp
/* Author Andrew Rudasics
/* Email: andrew.rudasics@digipen.edu
/* Date 9/20/2022
/* Implementation of camera class that holds necessary camera rendering data.
/* DigiPen Institute of Technology © 2022
/******************************************************************************/

#include "stdafx.h"
#include "InputAction.h"
#include "RenderCam.h"

#include "InputManager.h"
#include "ScriptingManager.h"
#include "Behaviour.h"

RenderCam::RenderCam(Int32 width, Int32 height, const char* controls_config) :
    m_position(glm::vec3(0)), m_direction(glm::vec3(0, 0, -1.0f)), 
    m_near_clip(0.1f), m_far_clip(1000.0f), m_fov(45.0f), 
    m_width(width), m_height(height), 
    camera_right(glm::vec3(0)), camera_up(glm::vec3(0)),
    camera_center(glm::vec3(0)),
    camera_spin(160.0f), camera_tilt(35.0f), camera_zoom(20.0f), camera_speed(0.5f),
    y_axis_inversion(false),
    m_projection_mode(ProjectionMode::PERSPECTIVE),
    m_ortho_size(10), m_control_mode(ControlMode::VIEWER),
    action_map(controls_config), p_behavior(nullptr) {

}

RenderCam::RenderCam(Int32 width, Int32 height, 
    const char* script, const char* controls_config) :
    m_position(glm::vec3(0)), m_direction(glm::vec3(0, 0, -1.0f)),
    m_near_clip(0.1f), m_far_clip(1000.0f), m_fov(45.0f),
    m_width(width), m_height(height),
    camera_right(glm::vec3(0)), camera_up(glm::vec3(0)),
    camera_center(glm::vec3(0)),
    camera_spin(160.0f), camera_tilt(35.0f), camera_zoom(20.0f), camera_speed(0.5f),
    y_axis_inversion(false),
    m_projection_mode(ProjectionMode::PERSPECTIVE),
    m_ortho_size(10), m_control_mode(ControlMode::VIEWER),
    action_map(controls_config) {
    Behaviour* bh = p_scripting_manager->CreateBehaviour(this);
    bh->AddScript(script);
    p_behavior = bh;
    bh->LoadScripts();
}

RenderCam::~RenderCam() {}

//Handle inputs to update camera position and orientation
void RenderCam::Update(Float32 dt) {
    if ((ControlMode::NONE) & m_control_mode)
        return;

    Vec2 mouse_delta = p_input_manager->GetMouseDelta();
    float inversion_multiplier = 1.0f;

    if (y_axis_inversion) {
        inversion_multiplier = -1.0f;
    }

    if ((ControlMode::FIRST_PERSON | ControlMode::THIRD_PERSON) & m_control_mode) {
        //Update camera center
        Vec3 move_dir = Vec3(0);

        if (action_map.IsActionPressed(InputAction::Actions::LEFT))
            move_dir -= glm::normalize(GetCameraRight());
        if (action_map.IsActionPressed(InputAction::Actions::RIGHT))
            move_dir += glm::normalize(GetCameraRight());

        Vec3 look_dir = GetLookDirection();
        Vec3 look_dir_proj = glm::normalize(Vec3(look_dir.x, 0, look_dir.z));

        if (action_map.IsActionPressed(InputAction::Actions::UP))
        {
            move_dir += look_dir_proj;
        }
        if (action_map.IsActionPressed(InputAction::Actions::DOWN))
        {
            move_dir -= look_dir_proj;
        }

        camera_center += 0.05f * move_dir;
    }
    // Look Vector
    Vec2 d_look_stick = Vec2(0);
    
    if ((ControlMode::FIRST_PERSON | ControlMode::THIRD_PERSON) & m_control_mode)
    {
        if (action_map.IsActionPressed(InputAction::Actions::UP_ALT)) {
            d_look_stick.y += inversion_multiplier * 1.0f;
        }
        if (action_map.IsActionPressed(InputAction::Actions::DOWN_ALT)) {
            d_look_stick.y -= inversion_multiplier * 1.0f;
        }
        if (action_map.IsActionPressed(InputAction::Actions::LEFT_ALT)) {
            d_look_stick.x -= 1.0f;
        }
        if (action_map.IsActionPressed(InputAction::Actions::RIGHT_ALT)) {
            d_look_stick.x += 1.0f;
        }
    }

    if ((ControlMode::FIRST_PERSON | ControlMode::THIRD_PERSON | ControlMode::VIEWER) & m_control_mode)
    {
        // Reset Camera
        if (action_map.IsActionTriggered(InputAction::Actions::ACTION_SELECT))
        {
            ResetDefaults();
        }
        Vec2 d_look_mouse = Vec2(0);
        if (action_map.IsActionPressed(InputAction::Actions::ACTION_1))
        {
            if (action_map.IsActionPressed(InputAction::Actions::ACTION_2))
            {
                // Pan Camera
                camera_center -= 0.05f * mouse_delta.x * glm::normalize(GetCameraRight());
                camera_center += 0.05f * inversion_multiplier * mouse_delta.y * Vec3(0, 1, 0);
            }
            else
            {
                d_look_mouse = Vec2(mouse_delta.x, inversion_multiplier * mouse_delta.y);
            }
        }

        Vec2 d_look = d_look_stick + d_look_mouse;

        // Rotate Camera
        camera_tilt = std::clamp(camera_tilt - camera_speed * d_look.y, -89.9f, 89.9f);
        if (ControlMode::FIRST_PERSON & m_control_mode)
            camera_spin -= camera_speed * d_look.x;
        else
            camera_spin += camera_speed * d_look.x;
    }

    // Zoom Camera
    if ((ControlMode::VIEWER | ControlMode::TOP_DOWN) & m_control_mode) {
        if (action_map.IsActionPressed(InputAction::Actions::ACTION_3))
        {
            camera_zoom *= 0.99f;
        }
        else if (action_map.IsActionPressed(InputAction::Actions::ACTION_4))
        {
            camera_zoom *= 1.01f;
        }
    }
   

    Mat4 spin_tr = glm::rotate(Mat4(1), glm::radians(camera_spin), Vec3(0, 1, 0));
    Mat4 tilt_tr = glm::rotate(Mat4(1), glm::radians(camera_tilt), Vec3(1, 0, 0));
    if (ControlMode::FIRST_PERSON & m_control_mode) {
        SetPosition(camera_center);

        Vec3 new_dir = spin_tr * tilt_tr * Vec4(0, 0, -1, 1);
        SetLookDirection(glm::normalize(new_dir));
    }
    if (ControlMode::TOP_DOWN & m_control_mode) {
        SetPosition(camera_center + Vec3(0.0, 1.0 * camera_zoom, 0.0));
        
        SetLookDirection(Vec3(0.0, -1.0, 0.0), Vec3(1.0, 0.0, 0.0));
    }
    else {
        Mat4 zoom_tr = glm::translate(Mat4(1), Vec3(0, 0, -camera_zoom));
        Mat4 ctr_tr = glm::translate(Mat4(1), camera_center);

        Vec4 new_pos = ctr_tr * spin_tr * tilt_tr * zoom_tr * glm::vec4(0, 0, 0, 1);
        SetPosition(new_pos);
        SetLookDirection(normalize(camera_center - Vec3(new_pos)));
    }
    
    if (p_behavior != nullptr)
        p_behavior->Update(dt);
}

std::pair<Vec3, Vec3> RenderCam::ScreenPointToRay(Ivec2 const& screen_pt) const {

   
    Mat4 const invProj = glm::inverse(GetProjMat());
    Mat4 const invView = glm::inverse(GetViewMat());

    // Convert mouse screen coords to homogeneous clip coords
    Vec4 const rayClip{
        (2.0f * screen_pt.x) / m_width - 1.0f,
        1.0f - (2.0f * screen_pt.y) / m_height,
        -1.0f,
        1.0f
    };

    // Convert clip space to camera space
    Vec4 rayCam = invProj * rayClip;
    rayCam.z = -1.0f;

    // Convert ray in camera space to world space
    if (m_projection_mode == ProjectionMode::PERSPECTIVE) {

        rayCam.w = 0.0f;
        return { m_position, glm::normalize(Vec3(invView * rayCam)) };
    }
    else {

        rayCam.w = 1.0f;
        return{ Vec3(invView * rayCam), m_direction };
    }
}

/*
* Provides the view matrix for the camera
* Returns: Mat4 view matrix of camera
*/
Mat4 RenderCam::GetViewMat() const {
	return glm::lookAt(m_position, m_position + m_direction, camera_up);
}

/*
* Provides the projection matrix for the camera
* Returns: Mat4 projection matrix of camera
*/
Mat4 RenderCam::GetProjMat() const {
    if (m_projection_mode == ProjectionMode::PERSPECTIVE){
        return glm::perspectiveFov(glm::radians(m_fov), (float)m_width, (float)m_height, m_near_clip, m_far_clip);
    }
    else
    {
        float aspect = ((float)m_width) / ((float)m_height);
        float half_size = (float)(m_ortho_size / 2);
        
        return glm::ortho(-aspect * half_size, aspect * half_size, -half_size, half_size, m_near_clip, m_far_clip);
    }
}

/*
* Provides the concatenation of the view and projection matrices
* Returns: Mat4 concatenated proj * view matrix of camera
*/
Mat4 RenderCam::GetViewProjMat() const {
	return GetProjMat() * GetViewMat();
}

/*
* Updates projection matrix
*/
void RenderCam::UpdateProjectionMatrix()
{
    if (m_projection_mode == ProjectionMode::PERSPECTIVE)
    {
        m_projection_matrix = glm::perspectiveFov(glm::radians(m_fov), (float)m_width, (float)m_height, m_near_clip, m_far_clip);
    }
    else
    {
        float aspect = ((float)m_width) / ((float)m_height);
        float half_size = (float)(m_ortho_size / 2);

        m_projection_matrix = glm::ortho(-aspect * half_size, aspect * half_size, -half_size, half_size, m_near_clip, m_far_clip);
    }
}

// ******************* Setters ********************

/*
* Sets the aspect ratio of the camera
* Param: Int32 width of camera 
* Param: Int32 height of camera
*/
void RenderCam::SetAspect(Int32 width, Int32 height) {
	m_width = width;
	m_height = height;
    UpdateProjectionMatrix();
}

/*
* Sets the position of the camera
* Param: Vec3 new camera position
*/
void RenderCam::SetPosition(Vec3 pos) {
	m_position = pos;
    UpdateProjectionMatrix();
}

/*
* Sets the position of the camera center of focus
* Param: Vec3 new camera focus object position
*/
void RenderCam::SetCameraCenter(Vec3 pos) {
    camera_center = pos;
    UpdateProjectionMatrix();
}

void RenderCam::SetZoom(Float32 _zoom) {
    camera_zoom = _zoom;
}

/*
* Sets the look direction of the camera
* Param: Vec3 new camera viewing direction
*/
void RenderCam::SetLookDirection(Vec3 dir, Vec3 up) {
	m_direction = dir;
	camera_right = glm::normalize(glm::cross(m_direction, up));
	camera_up = glm::normalize(glm::cross(camera_right, m_direction));
    UpdateProjectionMatrix();
}

/*
* Sets the distance from the camera to the near clipping plane 
* Param: Float32 near clip distance 
*/
void RenderCam::SetNearClipDistance(Float32 z_near) {
	m_near_clip = z_near;
    UpdateProjectionMatrix();
}

/*
* Sets the distance from the camera to the far clipping plane
* Param: Float32 near clip distance
*/
void RenderCam::SetFarClipDistance(Float32 z_far) {
	m_far_clip = z_far;
    UpdateProjectionMatrix();
}

/*
* Sets the field of view of the camera in degrees
* Param: Float32 angle of field of view in degrees
*/
void RenderCam::SetFOV(Float32 degrees) {
	m_fov = degrees;
    UpdateProjectionMatrix();
}

/*
* Sets the field of view of the camera in degrees
* Param: Float32 angle of field of view in degrees
*/
void RenderCam::SetOrthoSize(Float32 size) {
    m_ortho_size = size;
    UpdateProjectionMatrix();
}

/*
* Sets the projection mode of the camera
* Param: enum ProjectionMode mode of desired projection mode
*/
void RenderCam::SetProjectionMode(ProjectionMode mode) {
    
    if (mode == m_projection_mode)
    {
        return;
    }
    
    if (mode == ProjectionMode::PERSPECTIVE)
    {
        m_projection_mode = mode;
    } else if (mode == ProjectionMode::ORTHOGRAPHIC)
    {
        m_projection_mode = mode;
    }
    UpdateProjectionMatrix();
}

void RenderCam::SetControllerMode(ControlMode mode)
{
    m_control_mode = mode;
    if (ControlMode::ISOMETRIC & m_control_mode)
    {
        camera_spin = 45.0f;
        camera_tilt = 45.0f;
        SetProjectionMode(ProjectionMode::ORTHOGRAPHIC);
    }
    else if ((ControlMode::FIRST_PERSON | ControlMode::THIRD_PERSON) & m_control_mode)
    {
        SetProjectionMode(ProjectionMode::PERSPECTIVE);
    }
}

// ****************** Getters ********************

/*
* Returns: Ivec2 (width, height) of the camera
*/
Ivec2 RenderCam::GetAspect() const {
	return Ivec2(m_width, m_height);
}

/*
* Returns: Vec3 position of the camera
*/
Vec3 RenderCam::GetPosition() const {
	return m_position;
}

/*
* Returns: Vec3 position of the camera center of focus
*/
Vec3 RenderCam::GetCameraCenter() const {
    return camera_center;
}

/*
* Returns: Vec3 viewing direction of the camera
*/
Vec3 RenderCam::GetLookDirection() const {
	return m_direction;
}

Vec3 RenderCam::GetCameraRight() const
{
	return camera_right;
}

Vec3 RenderCam::GetCameraUp() const
{
	return camera_up;
}

/*
* Returns: Float32 near clipping plane distance
*/
Float32 RenderCam::GetNearClipDistance() const {
	return m_near_clip;
}

/*
* Returns: Float32 far clipping plane distance
*/
Float32 RenderCam::GetFarClipDistance() const {
	return m_far_clip;
}

/*
* Resets render cam to far clip distance
*/
void RenderCam::ResetDefaults() {
    camera_spin = 160.0f;
    camera_tilt = 35.0f;
    camera_zoom = 20;
    camera_speed = 0.5f;
    camera_center = Vec3(0);
    
    m_fov = 45.0f;
}

Behaviour* RenderCam::GetBehaviourPtr() const {
    return p_behavior;
}

void RenderCam::SetBehaviourPtr(Behaviour* p_bh) {
    p_behavior = p_bh;
}

/*
* Returns: Float32 field of view angle in degrees
*/
Float32& RenderCam::GetFOV() {
	return m_fov;
}

/*
* Returns: Float32 size equal to height of the orthographic frustum
* width is size * screen aspect ratio
*/
Float32& RenderCam::GetOrthoSize() {
    return m_ortho_size;
}

/*
* Returns Current Projection mode
*/
ProjectionMode& RenderCam::GetProjectionMode() {
    return m_projection_mode;
}

Float32 RenderCam::GetZoom() {
    return camera_zoom;
}
