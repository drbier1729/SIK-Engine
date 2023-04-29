#include "stdafx.h"

#include "Engine/MemoryManager.h"
#include "Engine/GUIObjectManager.h"
#include "Engine/GraphicsManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/RenderCam.h"
#include "Engine/Rigidbody.h"
#include "Engine/MotionProperties.h"
#include "Engine/GameObject.h"
#include "Engine/Factory.h"
#include "Engine/GameStateManager.h"
#include "Engine/AudioManager.h"
#include "Engine/ResourceManager.h"

#include "GarageState.h"
#include "GamePlayState.h"
#include "BaseState.h"
#include "FollowCam.h"
#include "CarController.h"
#include "Inventory.h"
#include "FadeState.h"
#include "PromptState.h"
#include "ScriptInterface.h"
#include "PlayerCharacter.h"
#include "BallnChain.h"
#include "Attachment.h"
#include "Magnet.h"

GarageState::GarageState() : 
	GameState("GarageScene.json"), rotate_time(0.0f),
	camera(nullptr), prev_camera(nullptr), 
	p_construction_state(nullptr), p_musclebeach_state(nullptr),
	p_junkyard_state(nullptr), p_tutorial_state(nullptr),
	p_wb_obj{nullptr},
	wrecking_ball_upgraded(false),
	p_magnet_obj{nullptr},
	magnet_upgraded(false),
	player_orig_scale_tr{ 0.0f },
	magnet_orig_offset{ 0.0f },	magnet_orig_scale_tr{ 0.0f },
	first_start(true), p_prompt_state(nullptr),
	p_fade_in(nullptr), p_fade_out(nullptr), fading(false) {
	camera = p_memory_manager->GetDefaultAllocator().new_object<RenderCam>(
		p_graphics_manager->GetWindowWidth(), p_graphics_manager->GetWindowHeight(),
		"garage_camera.lua", "camera_controls.json");

	hud_objects_begin = p_gui_object_manager->CreateGUIFromFile("garage_HUD_begin.json");

	p_fade_out = p_memory_manager->GetDefaultAllocator().new_object<FadeState>("load_fade.json", 1.5f, true);
	p_fade_in = p_memory_manager->GetDefaultAllocator().new_object<FadeState>("load_fade.json", 1.5f, false);

	if (p_construction_state == nullptr) {
		p_construction_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("ConstructionArena.json", "CONSBACK"_sid);
		p_construction_state->DisableAllGameObjects();
		p_construction_state->Exit();
	}

	if (p_musclebeach_state == nullptr) {
#ifdef _DEBUG
		p_musclebeach_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("MuscleBeachArenaDebug.json", "BEACHBACK"_sid);
#else
		p_musclebeach_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("muscle_beach.json", "BEACHBACK"_sid);
#endif
		p_musclebeach_state->DisableAllGameObjects();
		p_musclebeach_state->Exit();
	}

	if (p_junkyard_state == nullptr) {
#ifdef _DEBUG
		p_junkyard_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("DebugScene.json", "JUNKBACK"_sid);
#else
		p_junkyard_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("junkyard_new.json", "JUNKBACK"_sid);
#endif
		p_junkyard_state->DisableAllGameObjects();
		p_junkyard_state->Exit();
	}

	if (p_tutorial_state == nullptr) {
#ifdef _DEBUG
		p_tutorial_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("DebugScene.json", "TUTBACK"_sid);
#else
		p_tutorial_state = p_memory_manager->GetDefaultAllocator().
			new_object<GamePlayState>("junkyard_final.json", "TUTBACK"_sid, false);
#endif
		p_tutorial_state->DisableAllGameObjects();
		p_tutorial_state->Exit();
		p_base_state->SetGamePlayState(p_tutorial_state);
	}

	if (p_prompt_state == nullptr) {
		p_prompt_state = p_memory_manager->GetDefaultAllocator().new_object<PromptState>();
		p_prompt_state->Exit();
		p_prompt_state->SetJunkyardState(p_tutorial_state);
	}
	p_fade_in->Exit();
	p_fade_out->Exit();

	camera->SetPosition(Vec3(16.352, 2.127, -14.542));
	camera->SetControllerMode(ControlMode::NONE);
	camera->SetCameraCenter(Vec3(0.0f));
	camera->SetZoom(30.0f);
}

GarageState::~GarageState() {
	p_memory_manager->GetDefaultAllocator().delete_object(camera);
	p_memory_manager->GetDefaultAllocator().delete_object(p_construction_state);
	p_memory_manager->GetDefaultAllocator().delete_object(p_musclebeach_state);
	p_memory_manager->GetDefaultAllocator().delete_object(p_junkyard_state);
	p_memory_manager->GetDefaultAllocator().delete_object(p_fade_in);
	p_memory_manager->GetDefaultAllocator().delete_object(p_fade_out);
	p_memory_manager->GetDefaultAllocator().delete_object(p_prompt_state);
	p_memory_manager->GetDefaultAllocator().delete_object(p_tutorial_state);
}

void GarageState::Enter() {
	prev_camera = p_graphics_manager->GetPActiveCam();
	p_graphics_manager->SetPActiveCam(camera);
	
	if (not first_start) {
		fading = false;
		p_fade_out->Enter();
		p_fade_in->Exit();
	}
	
	for (auto& obj : objects_in_current_state)
		obj->Enable();

	for (auto& obj : hud_objects_begin)
		obj->Enable();

	GameObject* player_obj = p_base_state->GetPlayerGameObjPtr();

	RigidBody* p_rb = player_obj->HasComponent<RigidBody>();
	p_rb->position = Vec3(-7.0f, 0.0f, 3.0f);
	p_rb->orientation = glm::angleAxis(glm::radians(120.0f), Vec3(0.0f, 1.0f, 0.0f));
	p_rb->motion_props->linear_velocity = Vec3(0);
	p_rb->motion_props->angular_velocity = Vec3(0);
		
	FollowCam* fc = player_obj->HasComponent<FollowCam>();
	fc->SetEnable(false);

	CarController* p_cc = player_obj->HasComponent<CarController>();
	p_cc->Disable();

	if (wrecking_ball_upgraded) {
		BallnChain* ballnchain = p_wb_obj->HasComponent<BallnChain>();
		Vec3 anchor_offset = ballnchain->GetAnchorOffset();
		ballnchain->SetAnchorOffset(anchor_offset * 3.0f);
		ballnchain->Reset();
	}

	if (magnet_upgraded) {
		// double the offset since player has been scaled
		Attachment* p_att = p_magnet_obj->HasComponent<Attachment>();
		p_att->SetOffset(magnet_orig_offset * 2.0f);

		// double scale
		Transform* p_tr = p_magnet_obj->HasComponent<Transform>();
		p_tr->scale = magnet_orig_scale_tr * 2.0f;

		// sound effects
		Magnet* p_magnet = p_magnet_obj->HasComponent<Magnet>();
		p_magnet->SetNumAttached(0);
		p_audio_manager->Stop(p_magnet->GetSoundID());
	}

	Transform* p_tr = player_obj->HasComponent<Transform>();
	
	//If it's a first start then launch the junkyard scene with the tutorial
	if (first_start) {
		// get player original scale
		player_orig_scale_tr = p_tr->scale;
		p_tr->scale = player_orig_scale_tr * 2.0f;

		RegisterPlayerToScripts();
		first_start = false;
		EnterTutorialArena();
		p_gamestate_manager->PushState(p_prompt_state);
	}
	else {
		p_tr->scale = player_orig_scale_tr * 2.0f;
	}

	p_graphics_manager->exposure = 2.0f;
}

void GarageState::Exit() {
	if (prev_camera != nullptr)
		p_graphics_manager->SetPActiveCam(prev_camera);

	for (auto& obj : hud_objects_begin)
		obj->Disable();

	GameObject* player_obj = p_base_state->GetPlayerGameObjPtr();

	CarController* p_cc = player_obj->HasComponent<CarController>();
	p_cc->Enable();

	if (wrecking_ball_upgraded) {
		BallnChain* ballnchain = p_wb_obj->HasComponent<BallnChain>();
		Vec3 anchor_offset = ballnchain->GetAnchorOffset();
		ballnchain->SetAnchorOffset(anchor_offset / 3.0f);
		ballnchain->Reset();
	}

	if (magnet_upgraded) {
		// reset the offset since player has been scaled
		Attachment* p_att = p_magnet_obj->HasComponent<Attachment>();
		p_att->SetOffset(magnet_orig_offset);

		// reset scale
		Transform* p_tr = p_magnet_obj->HasComponent<Transform>();
		p_tr->scale = magnet_orig_scale_tr;

		// sound effects
		Magnet* p_magnet = p_magnet_obj->HasComponent<Magnet>();
		p_magnet->PlayLoadedSound(true);
		p_magnet->SetNumAttached(0);
	}

	p_gui_object_manager->RemoveAllHighlightObjects();
	p_fade_in->Exit();
}

void GarageState::Update(Float32 dt) {
	if (fading) {
		if (not p_fade_in->IsComplete()) {
			p_fade_in->Update(dt);
		}
		else {
			fading = false;
			//p_fade_in->Exit();
			StartArena();
		}
		return;
	}
	if (not p_fade_out->IsComplete())
		p_fade_out->Update(dt);

	p_base_state->Update(dt);
	GameState::Update(dt);
	camera->Update(dt);
	HUDUpdate(dt);
	
	CheckLoseCondition();
}

void GarageState::FixedUpdate(Float32 fixed_timestep) {
	RotateCar(fixed_timestep);
	p_base_state->FixedUpdate(fixed_timestep);
	GameState::FixedUpdate(fixed_timestep);
}

Bool GarageState::HasWreckingBallUpgrade() const {
	return wrecking_ball_upgraded;
}

Bool GarageState::HasMagnetUpgrade() const {
	return magnet_upgraded;
}

Bool GarageState::CompletedJunkyard() const {
	return p_junkyard_state->GetProgress() >= Progress::Complete;
}
Bool GarageState::CompletedConstruction() const {
	return p_construction_state->GetProgress() >= Progress::Complete;
}
Bool GarageState::CompletedMuscleBeach() const {
	return p_musclebeach_state->GetProgress() >= Progress::Complete;
}

void GarageState::UpgradeWreckingBall() {
	p_wb_obj = p_factory->BuildGameObject("WreckingBall.json");
	p_base_state->AddGameObject(p_wb_obj);

	BallnChain* ballnchain = p_wb_obj->HasComponent<BallnChain>();
	Vec3 anchor_offset = ballnchain->GetAnchorOffset();
	ballnchain->SetAnchorOffset(anchor_offset * 3.0f);
	ballnchain->Reset();

	wrecking_ball_upgraded = true;
}

void GarageState::UpgradeMagnet() {
	p_magnet_obj = p_factory->BuildGameObject("Magnet.json");
	p_base_state->AddGameObject(p_magnet_obj);

	// double the offset since player has been scaled
	Attachment* p_attach = p_magnet_obj->HasComponent<Attachment>();
	magnet_orig_offset = p_attach->GetOffset();
	p_attach->SetOffset(magnet_orig_offset * 2.0f);

	// double the scale
	Transform* p_tr = p_magnet_obj->HasComponent<Transform>();
	magnet_orig_scale_tr = p_tr->scale;
	p_tr->scale = magnet_orig_scale_tr * 2.0f;

	// sound effects
	Magnet* p_magnet = p_magnet_obj->HasComponent<Magnet>();
	p_magnet->PlayLoadedSound(false);
	p_magnet->SetNumAttached(0);

	magnet_upgraded = true;
}

void GarageState::EnterConstructionArena() {
	fading = true;
	p_fade_in->Enter();
	p_base_state->SetGamePlayState(p_construction_state);
}

void GarageState::EnterJunkyardArena() {
	fading = true;
	p_fade_in->Enter();
	p_base_state->SetGamePlayState(p_junkyard_state);
}

void GarageState::EnterMuscleArena() {
	fading = true;
	p_fade_in->Enter();
	p_base_state->SetGamePlayState(p_musclebeach_state);
}

void GarageState::EnterTutorialArena() {
	RigidBody* p_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();
	p_rb->position = Vec3(-75.0f, 0.0f, -95.0f);
	p_rb->orientation = glm::angleAxis(glm::radians(0.0f), Vec3(0.0f, 1.0f, 0.0f));
	Transform* p_tr = p_base_state->GetPlayerGameObjPtr()->HasComponent<Transform>();
	p_tr->scale = player_orig_scale_tr;

	CarController* p_cc = p_base_state->GetPlayerGameObjPtr()->HasComponent<CarController>();
	p_cc->SetAngle(180.0f);

	for (auto& obj : objects_in_current_state)
		obj->Disable();

	p_base_state->SetGamePlayState(p_tutorial_state);
	p_gamestate_manager->PushState(p_tutorial_state);

	p_graphics_manager->exposure = 2.0f;
}

void GarageState::StartConstructionArena() {
	RigidBody* p_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();
	p_rb->position = Vec3(-107.0f, 0.0f, -54.0f);
	p_rb->orientation = glm::angleAxis(glm::radians(0.0f), Vec3(0.0f, 1.0f, 0.0f));
	Transform* p_tr = p_base_state->GetPlayerGameObjPtr()->HasComponent<Transform>();
	p_tr->scale = player_orig_scale_tr;

	for (auto& obj : objects_in_current_state)
		obj->Disable();

	p_base_state->SetGamePlayState(p_construction_state);
	p_gamestate_manager->PushState(p_construction_state);

	p_graphics_manager->SetPrimaryLightPosition(Vec3(50, 100, 50));
	p_graphics_manager->exposure = 2.0f;
}

void GarageState::StartJunkyardArena() {
	RigidBody* p_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();

	p_rb->position = Vec3(-54.0f, 0.0f, 272.0f);
	p_rb->orientation = glm::angleAxis(glm::radians(90.0f), Vec3(0.0f, 1.0f, 0.0f));

	Transform* p_tr = p_base_state->GetPlayerGameObjPtr()->HasComponent<Transform>();
	p_tr->scale = player_orig_scale_tr;

	CarController* p_cc = p_base_state->GetPlayerGameObjPtr()->HasComponent<CarController>();
	p_cc->SetAngle(-45.0f);

	for (auto& obj : objects_in_current_state)
		obj->Disable();

	p_gamestate_manager->PushState(p_junkyard_state);

	p_graphics_manager->SetPrimaryLightPosition(Vec3(-50, 100, 250));
	p_graphics_manager->exposure = 2.0f;
}

void GarageState::StartMuscleArena() {
	RigidBody* p_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();
	p_rb->position = Vec3(96.0f, 0.0f, 118.0f);
	p_rb->orientation = glm::angleAxis(glm::radians(0.0f), Vec3(0.0f, 1.0f, 0.0f));
	Transform* p_tr = p_base_state->GetPlayerGameObjPtr()->HasComponent<Transform>();
	p_tr->scale = player_orig_scale_tr;

	for (auto& obj : objects_in_current_state)
		obj->Disable();

	p_musclebeach_state->SetLastLevel(true);
	p_base_state->SetGamePlayState(p_musclebeach_state);
	p_gamestate_manager->PushState(p_musclebeach_state);

	p_graphics_manager->SetPrimaryLightPosition(Vec3(50, 100, 50));
	p_graphics_manager->exposure = 2.0f;
}

void GarageState::StartArena() {
	if (p_base_state->GetGamePlayState() == p_junkyard_state) {
		StartJunkyardArena();
	}
	else if (p_base_state->GetGamePlayState() == p_construction_state) {
		StartConstructionArena();
	}
	else if (p_base_state->GetGamePlayState() == p_musclebeach_state) {
		StartMuscleArena();
	}
}

void GarageState::HUDUpdate(Float32 dt) {
	p_gui_object_manager->UpdateUIControls(dt);
	for (auto& obj : hud_objects_begin)
		obj->Update(dt);
}

void GarageState::RegisterPlayerToScripts() { 
	GameObject* p_player_go = p_base_state->GetPlayerGameObjPtr();
	Behaviour* player_bh = p_player_go->HasComponent<Behaviour>();
	if (player_bh) {
		for (auto& script : player_bh->scripts) {
			RegisterLuaGarageState(script.script_state,
				this);
		}
	}

	for (auto& game_object : objects_in_current_state) {
		Behaviour* bh = game_object->HasComponent<Behaviour>();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaGarageState(script.script_state,
					this);
			}
		}
	}

	for (auto& gui_object : hud_objects_begin) {
		Behaviour* bh = gui_object->GetBehaviour();
		if (bh) {
			for (auto& script : bh->scripts) {
				RegisterLuaPlayerObject(script.script_state,
					p_player_go, "player_object");

				RegisterLuaGarageState(script.script_state,
					this);
			}
		}

		for (auto& embedded_obj : gui_object->GetEmbeddedObjectsRecursive()) {
			Behaviour* e_bh = embedded_obj->GetBehaviour();
			if (e_bh) {
				for (auto& script : e_bh->scripts) {
					RegisterLuaPlayerObject(script.script_state,
						p_player_go, "player_object");

					RegisterLuaGarageState(script.script_state,
						this);
				}
			}
		}
	}
}

void GarageState::RotateCar(float dt) {
	Float32 time_for_rotation = 10.0f;
	rotate_time += dt;
	
	if (rotate_time > time_for_rotation)
		rotate_time = 0.0f;

	Float32 rotate_angle = (rotate_time / time_for_rotation) * glm::two_pi<Float32>();
	RigidBody* p_rb = p_base_state->GetPlayerGameObjPtr()->HasComponent<RigidBody>();
	p_rb->orientation = glm::angleAxis(rotate_angle, Vec3(0.0f, 1.0f, 0.0f));
}

void GarageState::CheckLoseCondition() {
	GamePlayState* curr_gameplay_state = p_base_state->GetGamePlayState();

	if (curr_gameplay_state->GetProgress() == Progress::Lose) {
		GameObject* player_obj = p_base_state->GetPlayerGameObjPtr();
		PlayerCharacter* player_char = player_obj->HasComponent<PlayerCharacter>();
		
		//Heal by doing negative damage
		player_char->TakeDamage(-100);

		//Halve the resource count
		Inventory* player_inv = player_obj->HasComponent<Inventory>();
		Uint16 current_count = player_inv->GetCollectableCount(Collectable::CTypes::Resource1);
		player_inv->RemoveCollectables(Collectable::CTypes::Resource1, current_count / 2);

		//Reset the level
		curr_gameplay_state->ResetLevel();
		curr_gameplay_state->SetProgress(Progress::Working);
		curr_gameplay_state->DisableAllGameObjects();
		curr_gameplay_state->Exit();
		p_base_state->SetGamePlayState(curr_gameplay_state);
	}
}
