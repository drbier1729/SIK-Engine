#pragma once


/*
* Add the following code snippets to EngineMain.cpp. You should see two wireframe cubes, one of them
* static and the other moving from screen left to screen right. Warnings will be logged to the console
* on frames when the cubes intersect.
* 
* 
* 1) At top of file

         ///////////////////////////////////////////////////////////////////////////////
         // START TEST CODE
         #include "BoxGeometry.h"
         // END TEST CODE
         //////////////////////////////////////////////////////////////////////////////
*
* 2) In function EngineMain, before the while loop
    
        ////////////////////////////////////////////////////////////////////////
        // START TEST CODE
        GLuint dbg_shader = p_resource_manager->LoadShaderProgram(".\\Engine\\Assets\\Shaders\\standard\\debug");

        RigidBody* rb_A = p_physics_manager->CreateRigidBody();
        rb_A->SetFlag(RigidBody::Flag::IsValid, true);
        rb_A->SetMotionType(RigidBody::MotionType::Dynamic);
        rb_A->position = Vec3(-5, 0, 0);
        rb_A->bounds = { Vec3(1.0f) };
        MotionProperties* A_props = p_physics_manager->AddMotionProperties(rb_A);
        A_props->inverse_mass = 1.0f; // 1kg
        A_props->linear_drag = 0.0f; // drag coefficient
        A_props->gravity_scale = 0.0f; // set to 1.0f for Earth gravity (9.8 m/s^2)
        A_props->AddForce(Vec3(500, 0, 0)); // impulse in +x direction

        SIK_INFO("RB A position = {}, {}, {}", rb_A->position.x, rb_A->position.y, rb_A->position.z);


        RigidBody* rb_B = p_physics_manager->CreateRigidBody();
        rb_B->SetFlag(RigidBody::Flag::IsValid, true);
        rb_B->SetMotionType(RigidBody::MotionType::Static);
        rb_B->position = Vec3(0);
        rb_B->bounds = { Vec3(1.5f)};

        SIK_INFO("RB B position = {}, {}, {}", rb_B->position.x, rb_B->position.y, rb_B->position.z);

        Box box{};
        // END TEST CODE
        ////////////////////////////////////////////////////////////////////////
* 
* 3) After the call to p_graphics_manager->RenderScene in the while loop
    
        ////////////////////////////////////////////////////////////////////////
        // START TEST CODE
        p_physics_manager->DebugDraw(dbg_shader, p_graphics_manager->GetRenderCam(), extrapolation, box);
        // END TEST CODE
        ////////////////////////////////////////////////////////////////////////
*/