#pragma once
/****************************************************************************** /
/* !
/* File RenderCam.h
/* Author Andrew Rudasics
/* Email: andrew.rudasics@digipen.edu
/* Date 9/20/2022
/* Interface for camera class used to render to screen.
/* DigiPen Institute of Technology © 2022
/******************************************************************************/

#include "InputAction.h"

enum ProjectionMode
{
	PERSPECTIVE = 0,
	ORTHOGRAPHIC,
	NUM_ENTRIES
};

enum ControlMode
{
	NONE = 0,
	VIEWER = 1,
	FIRST_PERSON = 2,
	THIRD_PERSON = 4,
	ISOMETRIC = 8,
	TOP_DOWN = 16
};

class Behaviour;

/* 
* The RenderCam class encapsulates the behavior of a camera in the scene.
* Provides methods to adjust settings and retrieve camera data.
*/
class RenderCam {
private:
	//World Position, Look direction vector
	Vec3 m_position, m_direction;
	
	//Distance to near clippping plan, far clipping plane and field of view
	Float32 m_near_clip, m_far_clip, m_fov, m_ortho_size;
	
	//Width and height of the image plane
	Int32 m_width, m_height;

	//Camera right vector direction and up vector direction.
	Vec3 camera_right, camera_up;

	Vec3 camera_center;
	
	//Speeds for camera movement
	Float32 camera_spin, camera_tilt, camera_zoom, camera_speed;

	//Toggle for y-axis-inversion
	Bool y_axis_inversion;

	//Action map for camera controls
	InputAction action_map;

	//Controller Mode
	ControlMode m_control_mode;

	// Projection mode of camera matrix
	ProjectionMode m_projection_mode;

	Mat4 m_projection_matrix;

	void UpdateProjectionMatrix();

	Behaviour* p_behavior;
public:
	RenderCam(Int32 width, Int32 height, 
			  const char* controls_config="camera_controls.json");
	RenderCam(Int32 width, Int32 height, 
			  const char* script,
			  const char* controls_config="camera_controls.json");
	~RenderCam();

	//Handle inputs to update camera position and orientation
	void Update(Float32 dt);

	// Returns origin point and direction
	std::pair<Vec3, Vec3> ScreenPointToRay(Ivec2 const& screen_pt) const;

	Mat4 GetViewMat() const;
	Mat4 GetProjMat() const;
	Mat4 GetViewProjMat() const;

	void SetAspect(Int32 width, Int32 height);
	void SetPosition(Vec3 pos);
	void SetLookDirection(Vec3 dir, Vec3 up = glm::vec3(0.0, 1.0, 0.0));
	void SetNearClipDistance(Float32 z_near);
	void SetFarClipDistance(Float32 z_far);
	void SetFOV(Float32 degrees);
	void SetOrthoSize(Float32 size);
	void SetProjectionMode(ProjectionMode mode);
	void SetControllerMode(ControlMode mode);
	void SetCameraCenter(Vec3 center_pos);
	void SetZoom(Float32 _zoom);
	inline void SetYAxisInversion(Bool val) { y_axis_inversion = val; }

	Ivec2 GetAspect() const;
	Vec3 GetPosition() const;
	Vec3 GetLookDirection() const;
	Vec3 GetCameraRight() const;
	Vec3 GetCameraUp() const;
	Vec3 GetCameraCenter() const;
	Float32 GetNearClipDistance() const;
	Float32 GetFarClipDistance() const;
	Float32& GetFOV();
	Float32& GetOrthoSize();
	ProjectionMode& GetProjectionMode();
	Float32 GetZoom();
	inline Bool& GetYAxisInversion() { return y_axis_inversion; }

	void ResetDefaults();

	/*Function to Get/Set the behaviour pointer*/
	Behaviour* GetBehaviourPtr() const;
	void SetBehaviourPtr(Behaviour* p_bh);
};
