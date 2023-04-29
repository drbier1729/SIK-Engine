#include "stdafx.h"

#include "Engine/GameObject.h"
#include "Engine/PhysicsManager.h"
#include "Engine/GameObjectManager.h"
#include "Engine/Factory.h"
#include "Engine/Serializer.h"

#include "Anchored.h"

void Anchored::SetSegmentTransform(GameObject* prev_obj, GameObject* next_obj, GameObject* segment_object) {
	if (not prev_obj || not next_obj) { return; }

	if (not prev_obj->HasComponent<RigidBody>() ||
		not next_obj->HasComponent<RigidBody>() )
	{
		return;
	}

	Vec3 position_prev = prev_obj->HasComponent<RigidBody>()->position;
	Vec3 position_next = next_obj->HasComponent<RigidBody>()->position;

	Vec3 diff_vector = position_next - position_prev;
	Vec3 diff_vector_halved = diff_vector * 0.5f;
	Vec3 diff_vector_norm = glm::normalize(diff_vector);
	Vec3 segment_len(0, 0, 1);

	Quat orientation = glm::quat(segment_len, diff_vector_norm);

	Vec3 position = position_prev + diff_vector_halved;

	segment_object->HasComponent<Transform>()->orientation = orientation;
	segment_object->HasComponent<Transform>()->position = position;


}

void Anchored::ApplySpringForce(GameObject* prev_obj, GameObject* curr_obj, GameObject* next_obj) {
	Vec3 qi_prev, qi_next, qi_curr; //Positions
	Vec3 vi_prev, vi_next, vi_curr; //Velocities

	if (not prev_obj || not curr_obj || not next_obj) { return; }

	RigidBody* prev_rb = prev_obj->HasComponent<RigidBody>();
	RigidBody* curr_rb = curr_obj->HasComponent<RigidBody>();
	RigidBody* next_rb = next_obj->HasComponent<RigidBody>();

	if (not (prev_rb && prev_rb->motion_props) ||
		not (curr_rb && curr_rb->motion_props) ||
		not (next_rb && next_rb->motion_props))
	{
		return;
	}

	qi_prev = prev_rb->position;
	vi_prev = prev_rb->motion_props->linear_velocity;

	qi_curr = curr_rb->position;
	vi_curr = curr_rb->motion_props->linear_velocity;

	qi_next = next_rb->position;
	vi_next = next_rb->motion_props->linear_velocity;

	Vec3 diff = qi_prev - qi_curr;
	Vec3 n_diff = Vec3(0);
	Float32 diff_mag = glm::length(diff);
	if (diff_mag != 0.0f) {
		n_diff = glm::normalize(diff) * spring_rest_length;
	}

	Vec3 prev_force = (n_diff * (diff_mag - spring_rest_length)) * spring_coeff;


	diff = qi_next - qi_curr;
	n_diff = Vec3(0);
	diff_mag = glm::length(diff);
	if (diff_mag != 0.0f) {
		n_diff = glm::normalize(diff) * spring_rest_length;
	}
	Vec3 next_force = (n_diff * (diff_mag - spring_rest_length)) * spring_coeff;

	curr_rb->AddForce(prev_force + next_force);
}

void Anchored::SetSegmentTransforms() {

	SetSegmentTransform(
		anchor_point_obj, 
		chain_link_objects[0], 
		chain_segment_objects[0]);

	for (Uint8 i = 0; i < chain_links_count - 1; ++i) {
		SetSegmentTransform(
			chain_link_objects[i], 
			chain_link_objects[i + 1], 
			chain_segment_objects[i + 1]);
	}

	SetSegmentTransform(
		chain_link_objects[chain_links_count - 1],
		GetOwner(), 
		chain_segment_objects[chain_links_count]);
}

Anchored::Anchored() :
	chain_links_count{ 0 },
	spring_coeff{ 0.0f },
	damper_coeff{ 0.0f },
	spring_rest_length{ 0.0f },
	anchored_to_name{ nullptr },
	p_anchored_to_obj{ nullptr },
	anchor_point_obj{ nullptr },
	anchor_offset{ 0 } {

}

Anchored::~Anchored() noexcept {
	for (auto& obj : chain_link_objects) {
		p_game_obj_manager->DeleteGameObject(obj);
	}
	chain_link_objects.clear();

	for (auto& obj : chain_segment_objects) {
		p_game_obj_manager->DeleteGameObject(obj);
	}
	chain_segment_objects.clear();

	p_game_obj_manager->DeleteGameObject(anchor_point_obj);
}

void Anchored::Deserialize(rapidjson::Value const& json_value) {
	DeserializeReflectable<Anchored>(json_value, this);
	anchored_to_name = json_value.FindMember("anchored_to")->value.GetString();

	if (chain_links_count < 3) {
		SIK_WARN("Minimum number of chain links is 3. Setting the chain link count to 3");
		chain_links_count = 3;
	}

	chain_segment_objects.push_back(
		p_factory->BuildGameObject("AnchorSegment.json"));

	//Create an object for each chain link and maintain a list of them.
	for (Uint8 i = 0; i < chain_links_count; ++i) {
		chain_link_objects.push_back(
			p_factory->BuildGameObject("ChainLink.json"));

		chain_segment_objects.push_back(
			p_factory->BuildGameObject("AnchorSegment.json"));
	}

	//Create the anchor object which will always be at the 
	//specified offset position to the parent
	anchor_point_obj = p_factory->BuildGameObject("ChainAnchor.json");
}

void Anchored::Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) {
	SerializeReflectable<Anchored>(json_value, this, alloc);
}

void Anchored::Modify(rapidjson::Value const& json_value) {
	DeserializeReflectable<Anchored>(json_value, this);

	auto itr = json_value.FindMember("anchored_to");
	if (itr != json_value.MemberEnd())
		anchored_to_name = itr->value.GetString();

	if (chain_link_objects.size() < chain_links_count) {
		AddChainLink(chain_links_count - chain_link_objects.size());
	}
	else if (chain_link_objects.size() > chain_links_count) {
		RemoveChainLink(chain_link_objects.size() - chain_links_count);
	}
}

void Anchored::Link() {
	auto& go_pool = p_game_obj_manager->GetGameObjectContainer();

	for (auto r = go_pool.all(); not r.is_empty(); r.pop_front()) {
		GameObject& go_ref = r.front();
		if (go_ref.GetName().compare(anchored_to_name) == 0) {
			p_anchored_to_obj = &go_ref;
		}
	}

	if (p_anchored_to_obj == nullptr)
		return;

	Transform* anchor_tr = anchor_point_obj->HasComponent<Transform>();
	Transform* anchored_to_tr = p_anchored_to_obj->HasComponent<Transform>();
	anchor_tr->position = anchored_to_tr->position + anchor_offset;

	Float32 x_offset = spring_rest_length;

	for (Uint8 i = 0; i < chain_links_count; ++i) {
		GameObject* go = chain_link_objects[i];
		RigidBody* rb = go->HasComponent<RigidBody>();
		if (!rb) {	continue; }
		rb->position = anchored_to_tr->position;
		rb->position.x += x_offset * (i + 1);

		go = chain_segment_objects[i];
		Transform* tr = go->HasComponent<Transform>();
		tr->position = anchored_to_tr->position;
		tr->position.x += x_offset * (i + 1);
	}
	GameObject* go = chain_segment_objects[chain_links_count];
	Transform* tr = go->HasComponent<Transform>();
	tr->position = anchored_to_tr->position;
	tr->position.x += x_offset * (chain_links_count);
}

void Anchored::Enable() {
	for (auto& obj : chain_link_objects) {
		obj->Enable();
	}

	for (auto& obj : chain_segment_objects) {
		obj->Enable();
	}

	anchor_point_obj->Enable();
}

void Anchored::Disable() {
	for (auto& obj : chain_link_objects) {
		obj->Disable();
	}

	for (auto& obj : chain_segment_objects) {
		obj->Disable();
	}

	anchor_point_obj->Disable();
}

void Anchored::AddChainLink(Uint8 add_count) {
	Transform* anchored_to_tr = p_anchored_to_obj->HasComponent<Transform>();
	Float32 x_offset = spring_rest_length;

	for (Uint8 i = 0; i < add_count; ++i) {
		chain_link_objects.push_back(
			p_factory->BuildGameObject("ChainLink.json"));
		
		if (p_anchored_to_obj != nullptr) {
			RigidBody* rb = chain_link_objects.back()->HasComponent<RigidBody>();
			if (!rb) { continue;}
			rb->position.x = anchored_to_tr->position.x;
			rb->position.x += x_offset * (i + 1);
		}

		chain_segment_objects.push_back(
			p_factory->BuildGameObject("AnchorSegment.json"));
		if (p_anchored_to_obj != nullptr) {
			RigidBody* rb = chain_segment_objects.back()->HasComponent<RigidBody>();
			if (!rb) { continue; }
			rb->position.x = anchored_to_tr->position.x;
			rb->position.x += x_offset * (i + 1);
		}
	}
}

void Anchored::RemoveChainLink(Uint8 remove_count) {
	if (chain_link_objects.size() - remove_count < 3) {
		SIK_WARN("Chain link count cannot go below 3");
		remove_count = static_cast<Uint8>(chain_link_objects.size()) - 3;
	}
	GameObject* obj_to_remove;
	for (Uint8 i = 0; i < remove_count; ++i) {
		obj_to_remove = chain_link_objects.back();
		chain_link_objects.pop_back();
		p_game_obj_manager->DeleteGameObject(obj_to_remove);

		obj_to_remove = chain_segment_objects.back();
		chain_segment_objects.pop_back();
		p_game_obj_manager->DeleteGameObject(obj_to_remove);
	}

	chain_links_count = static_cast<Uint8>(chain_link_objects.size());
}

void Anchored::FixedUpdate(Float32 dt) {

	//Set the anchor point at the offset and apply the parent rotation
	RigidBody* anchor_rb = anchor_point_obj->HasComponent<RigidBody>();
	RigidBody* parent_rb = p_anchored_to_obj->HasComponent<RigidBody>();
	if (!anchor_rb || !parent_rb) { return; }
	Vec3 anchor_pos = parent_rb->LocalToWorld(anchor_offset);
	anchor_rb->position = anchor_pos;
	/*
	* Calculate the spring damper forces on each chain link
	*/

	//For the first link, use the parent object for position and velocity
	ApplySpringForce(
		anchor_point_obj,
		chain_link_objects[0],
		chain_link_objects[1]
	);

	//For the middle links we can use the surrounding links
	for (Uint8 i = 1; i < chain_links_count - 1; ++i) {
		ApplySpringForce(
			chain_link_objects[i - 1],
			chain_link_objects[i],
			chain_link_objects[i + 1]
		);
	}

	//For the last link, use the ball object for the position and velocity
	ApplySpringForce(
		chain_link_objects[chain_links_count - 2],
		chain_link_objects[chain_links_count - 1],
		GetOwner()
	);

	//Finally for the ball, we can consider the force only being applyied from the last link
	ApplySpringForce(
		chain_link_objects[chain_links_count - 1],
		GetOwner(),
		GetOwner()
	);
}

void Anchored::Update(Float32 dt) {
#ifdef _ENABLE_EDITOR
	if (static_cast<Uint8>(chain_link_objects.size()) < chain_links_count) {
		AddChainLink(chain_links_count - static_cast<Uint8>(chain_link_objects.size()));
	}
	else if (static_cast<Uint8>(chain_link_objects.size()) > chain_links_count) {
		RemoveChainLink(static_cast<Uint8>(chain_link_objects.size()) - chain_links_count);
	}
#endif

	SetSegmentTransforms();

	//Correct the scale according to the spring rest length
	for (auto& obj : chain_segment_objects) {
		obj->HasComponent<Transform>()->scale.z *= spring_rest_length;
	}
}


BEGIN_ATTRIBUTES_FOR(Anchored)
DEFINE_MEMBER(Uint8, chain_links_count)
DEFINE_MEMBER(Float32, spring_coeff)
DEFINE_MEMBER(Float32, damper_coeff)
DEFINE_MEMBER(Float32, spring_rest_length)
DEFINE_MEMBER(Vec3, anchor_offset)
END_ATTRIBUTES