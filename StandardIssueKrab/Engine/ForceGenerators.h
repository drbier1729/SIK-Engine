#pragma once

struct RigidBody;

struct ForceGenerator 
{
	// Adds force to rb->motion_props->accumulated_force
	virtual void operator()(RigidBody* rb, Float32 dt) const noexcept = 0;
};

struct ForceRegistration
{
	RigidBody* rb;
	ForceGenerator* fg;
};