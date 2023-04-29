#pragma once

#include "Engine\Component.h"
#include "Engine\GameObject.h"
#include "Engine\Mesh.h"
#include "Engine\ResourceManager.h"
#include "Engine\SIKTypes.h"

class ObjectiveTracker : public Component
{
public:
	VALID_COMPONENT(ObjectiveTracker)
	ObjectiveTracker();
	~ObjectiveTracker();
	void Deserialize(rapidjson::Value const& json_value);
	void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc);
	void Update(Float32 dt) override;

	void SetObjective(GameObject* objective);
	void AddObjective(GameObject* objective);
	void SetPlayer(GameObject* other);
	
private:
	GameObject* m_player, *m_objective;
	Vector<GameObject*> m_objectives;
	Float32 m_reached_radius;
	Uint32 m_objective_idx;
};