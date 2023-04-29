#include "stdafx.h"
#include "ObjectiveTracker.h"
#include "Engine\GameObjectManager.h"
#include "Engine\MeshRenderer.h"
#include "Engine/AudioManager.h"

ObjectiveTracker::ObjectiveTracker() {
	m_reached_radius = 5.0f;
	m_objective_idx = 0;
	m_player = nullptr;
	m_objective = nullptr;
}

ObjectiveTracker::~ObjectiveTracker() {

}

void ObjectiveTracker::Deserialize(rapidjson::Value const& json_value) {
}

void ObjectiveTracker::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
}

void ObjectiveTracker::Update(Float32 dt) {

	// Check if we have an objective
	// if no, hide compass
	// if yes, get objective's position and use it to caclulate rotation
	if (m_objective == nullptr)
	{
		if (m_objectives.size() == 0)
		{
			m_objective_idx = 0;
			
			// Hide Game Object Compass
			GameObject* me = this->GetOwner();
			MeshRenderer* mr = me->HasComponent<MeshRenderer>();
			if (mr != nullptr)
			{
				mr->enabled = false;
			}

			return;
		}
		else
		{
			m_objective = m_objectives[m_objective_idx];

			// Hide Game Object Compass
			GameObject* me = this->GetOwner();
			MeshRenderer* mr = me->HasComponent<MeshRenderer>();
			if (mr != nullptr)
			{
				mr->enabled = true;
			}

			MeshRenderer* obj_mr = m_objective->HasComponent<MeshRenderer>();
			if (obj_mr != nullptr)
			{
				obj_mr->enabled = true;
			}
		}		
	}

	if (m_player == nullptr)
	{
		// Find player
		auto& go_container = p_game_obj_manager->GetGameObjectContainer();
		for (auto r = go_container.all(); not r.is_empty(); r.pop_front()) {
			GameObject& game_obj = r.front();
			if (game_obj.GetName() == "PlayerObj")
			{
				m_player = &game_obj;
				return; // return and update next update call
			}
		}
		
		// if not found, return
		return;
	}

	// Update the position and rotation of the object
	GameObject* me = this->GetOwner();
	Transform* tr = me->HasComponent<Transform>();
	if (tr == nullptr)
	{
		return;
	}


	Transform* player_tr = m_player->HasComponent<Transform>();
	if (player_tr == nullptr)
	{
		return;
	}
	Vec3 player_pos = player_tr->position;
	tr->position = player_pos;

	// Rotation = 
	Transform* objective_tr = m_objective->HasComponent<Transform>();
	if (objective_tr == nullptr)
	{
		return;
	}

	Vec3 objective_pos = objective_tr->position;
	
	// Check if we are close to objective
	if (glm::distance2(player_pos, objective_pos) < (m_reached_radius * m_reached_radius))
	{
		MeshRenderer* obj_mr = m_objective->HasComponent<MeshRenderer>();
		if (obj_mr != nullptr)
		{
			obj_mr->enabled = false;
			p_audio_manager->PlaySoundEffect("BEEP");
		}

		m_objective_idx = (m_objective_idx + 1) % m_objectives.size();
		m_objective = m_objectives[m_objective_idx];

		obj_mr = m_objective->HasComponent<MeshRenderer>();
		if (obj_mr != nullptr)
		{
			obj_mr->enabled = true;
		}
	}

	Vec3 objective_dir = objective_tr->position - tr->position;

	objective_dir.y = 0;
	objective_dir = glm::normalize(objective_dir);

	float angle = acos(glm::dot(glm::normalize(Vec3(1, 0, 0)), objective_dir));
	Vec3 axis = glm::normalize(glm::cross(glm::normalize(Vec3(1, 0, 0)), objective_dir));

	tr->orientation = glm::angleAxis(angle, axis); // Quat(angle, Vec3(0, 1, 0));
	tr->scale = Vec3(1);
}

void ObjectiveTracker::SetObjective(GameObject* objective)
{
	if (objective != nullptr) {
		m_objective = objective;
	}
}

void ObjectiveTracker::AddObjective(GameObject* objective)
{
	if (objective != nullptr) {
		m_objectives.push_back(objective);
		MeshRenderer* mr = objective->HasComponent<MeshRenderer>();
		mr->enabled = false;
	}
}

void ObjectiveTracker::SetPlayer(GameObject* other)
{
	m_player = other;
}