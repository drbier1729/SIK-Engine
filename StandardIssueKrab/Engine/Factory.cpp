#include "stdafx.h"
#include "Factory.h"

#include "ResourceManager.h"
#include "MemoryManager.h"
#include "GameObjectManager.h"
#include "GraphicsManager.h"
#include "PhysicsManager.h"
#include "GameObject.h"
#include "Serializer.h"
#include "Component.h"
#include "Transform.h"
#include "TestComp.h"
#include "Behaviour.h"
#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "SkinnedMesh.h"
#include "Model.h"
#include "RigidBody.h"
#include "Collision.h"
#include "WorldEditor.h"

#include "ScriptingManager.h"

static Bool CreateEngineSideComponent(GameObject* go, StringID comp_id, rapidjson::Value const& rj_value);
static void ModifyEngineSideComponent(GameObject* go, StringID comp_id, rapidjson::Value const& rj_value);

Factory::Factory() {}

GameObject* Factory::BuildGameObject(const char* filename) {
	JSON* json{ p_resource_manager->LoadJSON(filename) };
	if (json == nullptr) {
		SIK_ERROR("JSON document has not yet been loaded: {}", filename);
		SIK_ASSERT(false, "JSON document has not yet been loaded.");
		return nullptr;
	}
	
	rapidjson::Document& doc = json->doc;

	GameObject* p_obj = BuildGameObject(doc);

	return p_obj;
}

GameObject* Factory::BuildGameObject(rapidjson::Document& val) {
	GameObject* p_obj = nullptr;
	rapidjson::Value::ConstMemberIterator doc_itr;
	
	doc_itr = val.FindMember("Archetype");
	if (doc_itr != val.MemberEnd()) {
		p_obj = BuildGameObject(doc_itr->value.GetString());
		
		// override object name in Archetype
		doc_itr = val.FindMember("Name");
		if (doc_itr != val.MemberEnd()) { 
			p_obj->SetName(doc_itr->value.GetString()); 
		}

		//Code for handling the modification of components
		doc_itr = val.FindMember("Components");
		if (doc_itr != val.MemberEnd()) {
			for (auto const& comp : doc_itr->value.GetObj()) {
				SIK_ASSERT(comp.name.IsString(), "Component name not found.");
				StringID comp_id = ToStringID(comp.name.GetString());
				Component* p_created_component = p_obj->HasComponent(comp_id);

				if (p_created_component) {
					p_created_component->Modify(comp.value);
				}
				else {
					ModifyEngineSideComponent(p_obj, comp_id, comp.value);
				}
			}
		}
	}
	else {
		doc_itr = val.FindMember("Name");
		SIK_ASSERT(doc_itr != val.MemberEnd(), "Game object name not found.");
	p_obj = p_game_obj_manager->CreateGameObject(doc_itr->value.GetString());

		doc_itr = val.FindMember("Components");
		if (doc_itr != val.MemberEnd()) {
			for (auto const& comp : doc_itr->value.GetObj()) {
				SIK_ASSERT(comp.name.IsString(), "Component name not found.");
				StringID comp_id = ToStringID(comp.name.GetString());
				auto it = builder_map.find(comp_id);

				if (it == builder_map.end()) {
					Bool success = CreateEngineSideComponent(p_obj, comp_id, comp.value);
					if (not success) {
						SIK_ERROR("Unable to build component {}", comp.name.GetString());
						SIK_ASSERT(success, "Unable to build component.");
					}
				}
				else {
					auto& builder = it->second;
					Component* p_created_component = builder();
					p_obj->AddComponent(p_created_component);
					p_created_component->Deserialize(comp.value);
				}
			}
		}
	}

	/*
	* Linking step
	* Links related components after they have all been added to the game object
	*/

	Behaviour* bh = p_obj->HasComponent<Behaviour>();
	if (bh)
		bh->LoadScripts();

	p_obj->Link();

	return p_obj;
}

// TODO : this will need to be updated for Engine-side Components after the Engine Milestone

// TODO : can replace first arg with a StringID since the file should already have been loaded
// before calling this function... OR we could make it so this function creates a new JSON
// object and even a new file if the name is not found.
void Factory::SaveObjectArchetype(const char* archetype_filename, GameObject const& go) {
	
	JSON* json = p_resource_manager->GetJSON(archetype_filename);
	if (json == nullptr) {
		SIK_ERROR("JSON document has not yet been loaded: {}", archetype_filename);
		SIK_ASSERT(false, "JSON document has not yet been loaded.");
		return;
	}

	rapidjson::Value comp_override_object;
	comp_override_object.SetObject();

	Vector<UniquePtr<Component>> const& component_map = go.GetComponentArray();
	for (auto it = component_map.begin(); it != component_map.end(); ++it) {
		if (*it != nullptr) {
			rapidjson::Value comp_object;
			comp_object.SetObject();
			it->get()->Serialize(comp_object, json->doc.GetAllocator());

			comp_override_object.AddMember(
				rapidjson::StringRef((*it)->GetName()), 
				comp_object, 
				json->doc.GetAllocator());
		}
	}

	auto doc_itr = json->doc.FindMember("Components");
	if (doc_itr != json->doc.MemberEnd()) {
		doc_itr->value.Swap(comp_override_object);
	}
	else {
		json->doc.AddMember("Components", comp_override_object, json->doc.GetAllocator());
	}

	if (not p_resource_manager->WriteJSONToDisk(archetype_filename)) {
		SIK_ERROR("Tried to save GameObject {} to file {} but failed.", go.GetName(), json->path.string().c_str());
		SIK_ASSERT(false, "");
	}
}

Vector<GameObject*> Factory::BuildScene(const char* filename) {
	JSON* json{ p_resource_manager->LoadJSON(filename) };
	GameObject* temp_obj = nullptr;
	Vector<GameObject*> to_return;
	to_return.clear();

	if (json == nullptr) {
		SIK_ERROR("JSON document has not yet been loaded: {}", filename);
		SIK_ASSERT(false, "JSON document has not yet been loaded.");
		return to_return;
	}

	rapidjson::Document& doc = json->doc;

	for (auto mem = doc.MemberBegin(); mem != doc.MemberEnd(); ++mem) {
		rapidjson::Document doc_itr = nullptr;
		doc_itr.CopyFrom(mem->value, doc.GetAllocator());
		temp_obj = BuildGameObject(doc_itr);
		to_return.push_back(temp_obj);
	}

	// update currently working scene in editor
	std::filesystem::path curr_scene = std::filesystem::current_path() / ".." / "StandardIssueKrab" / "Engine" / "Assets" / "JSON" / filename;
	p_world_editor->SetOpenFileName(curr_scene.string().c_str());

	return to_return;
}

void Factory::SaveObjectInScene(rapidjson::Document& doc, GameObject const& go)
{
}


static Bool CreateEngineSideComponent(GameObject* go, StringID comp_id, rapidjson::Value const& rj_value) {
	
	Bool success = true;
	switch (comp_id)
	{
	break; case "Transform"_sid:
	{
		DeserializeReflectable(rj_value, go->HasComponent<Transform>());
	}
	break; case "RigidBody"_sid:
	{
		RigidBodyCreationSettings rb_init{};
		DeserializeReflectable(rj_value, &rb_init);

		auto it = rj_value.FindMember("motion_type");
		if (it != rj_value.MemberEnd()) {
			StringID mt = ToStringID(it->value.GetString());
			if (mt == "Dynamic"_sid) {
				rb_init.motion_type = RigidBody::MotionType::Dynamic;
			}
			else if (mt == "Kinematic"_sid) {
				rb_init.motion_type = RigidBody::MotionType::Kinematic;
				SIK_WARN("Kinematic RigidBodies are not yet supported!");
			}
		}

		it = rj_value.FindMember("colliders");
		if (it != rj_value.MemberEnd()) {
			SIK_ASSERT(it->value.IsArray(), "Must provide colliders as an array.");
			auto i = 0u;
			for (auto&& c : it->value.GetArray()) {
				if (i == RigidBody::MAX_COLLIDERS) {
					SIK_WARN("Maximum colliders, {}, added. The rest will be ignored.", i);
					break;
				}
				
				ColliderCreationSettings opts{};
				DeserializeReflectable(c, &opts);
				
				auto c_itr = c.FindMember("type");
				SIK_ASSERT(c_itr != c.MemberEnd(), "Must specify a collider type: Sphere, Capsule, or Hull.");
				
				StringID ct = ToStringID(c_itr->value.GetString());
				if (ct == "Sphere"_sid) {
					opts.type = Collision::Collider::Type::Sphere;
				}
				else if (ct == "Capsule"_sid) {
					opts.type = Collision::Collider::Type::Capsule;
				}
				else if (ct == "Hull"_sid) {
					opts.type = Collision::Collider::Type::Hull;
				}
				else {
					SIK_ASSERT(false, "Invalid Collider type provided. Must be Sphere, Capsule, or Hull.");
				}

				c_itr = c.FindMember("parameters");
				if (c_itr != c.MemberEnd()) {
					switch (opts.type) {
						break; case Collision::Collider::Type::Sphere:{
							DeserializeReflectable(c_itr->value, &opts.sphere_args);
						}
						break; case Collision::Collider::Type::Capsule:{
							DeserializeReflectable(c_itr->value, &opts.capsule_args);
						}
						break; case Collision::Collider::Type::Hull:{
							DeserializeReflectable(c_itr->value, &opts.hull_args);
						}
					}
				}

				rb_init.collider_parameters[i++] = opts;
			}
		}

		// Create the RigidBody
		RigidBody* p_rb = p_physics_manager->CreateRigidBody(rb_init);
		go->AddComponent(p_rb);
	}
	break; case "MeshRenderer"_sid:
	{
		// renderer always needs a material
		auto it = rj_value.FindMember("material");
		SIK_ASSERT(it != rj_value.MemberEnd(), "material filepath required");
		SIK_ASSERT(it->value.IsString(), "material filename must be a string");
		Material* mat = p_resource_manager->LoadMaterial(it->value.GetString());

		if (mat == nullptr) { SIK_ERROR("Material failed to load {}.", it->value.GetString()); }
		SIK_ASSERT(mat != nullptr, "Material failed to load.");

		// renderer can have either a model...
		it = rj_value.FindMember("model");
		if (it != rj_value.MemberEnd()) {
			//Removing support for models since we don't use them.
		}
		// ... or a mesh
		else {
			it = rj_value.FindMember("mesh");
			SIK_ASSERT(it != rj_value.MemberEnd(), "mesh filepath required");
			SIK_ASSERT(it->value.IsString(), "mesh filename must be a string");
			Mesh* mesh = p_resource_manager->LoadMesh(it->value.GetString());

			if (mesh == nullptr) { SIK_ERROR("Mesh failed to load {}.", it->value.GetString()); }
			SIK_ASSERT(mesh != nullptr, "Mesh failed to load.");

			go->AddComponent(p_graphics_manager->CreateMeshRenderer(mat, mesh));
		}
	}
	break; case "Behaviour"_sid:
	{
		Behaviour* bh = p_scripting_manager->CreateBehaviour();

		auto it = rj_value.FindMember("scripts");
		SIK_ASSERT(it->value.IsArray(), "scripts should be in an array");
		for (auto& it2 : it->value.GetArray()) {
			SIK_ASSERT(it2.IsString(), "script filename must be a string");
			bh->AddScript(it2.GetString());
		}
		go->AddComponent(bh);

		/* Moved the LoadScripts() call from here
		* to after all the other components have been created
		*/
	}
	break; default: { success = false; }
	}

	return success;
}


static void ModifyEngineSideComponent(GameObject* go, StringID comp_id, rapidjson::Value const& rj_value) {
	switch (comp_id)
	{
	case "Transform"_sid:
	{
		Transform* go_transform = go->HasComponent<Transform>();
		DeserializeReflectable(rj_value, go_transform);
	}
	break; case "RigidBody"_sid:
	{
		RigidBody* go_rigidbody = go->HasComponent<RigidBody>();

		auto position_it = rj_value.FindMember("position");
		if (position_it != rj_value.MemberEnd()) {
			SIK_ASSERT(position_it->value.IsArray(), "rapidjson value is not an array.");
			auto const& source = position_it->value.GetArray();
			go_rigidbody->position.x = source[0].GetFloat();
			go_rigidbody->position.y = source[1].GetFloat();
			go_rigidbody->position.z = source[2].GetFloat();
		}

		auto orientation_it = rj_value.FindMember("orientation");
		//Orientation converts from Euler angle rotation to Quaternion
		//Uses rotation ordering - XYZ
		if (orientation_it != rj_value.MemberEnd()) {
			SIK_ASSERT(orientation_it->value.IsArray(), "rapidjson value is not an array.");
			auto const& source = orientation_it->value.GetArray();
			go_rigidbody->orientation = glm::quat(
				glm::vec3(
					glm::radians(source[0].GetFloat()),
					glm::radians(source[1].GetFloat()),
					glm::radians(source[2].GetFloat())
				)
			);
		}
	}
	break; case "Behaviour"_sid:
	{
		Behaviour* go_behaviour = go->HasComponent<Behaviour>();
		Bool create = false;
		if (go_behaviour == nullptr) {
			go_behaviour = p_scripting_manager->CreateBehaviour();
			create = true;
		}
		auto scripts_it = rj_value.FindMember("scripts");
		SIK_ASSERT(scripts_it->value.IsArray(), "scripts should be in an array");
		for (auto& script_name : scripts_it->value.GetArray()) {
			SIK_ASSERT(script_name.IsString(), "script filename must be a string");
			
			// check if adding existed script
			auto it = go_behaviour->scripts.begin();
			for (; it != go_behaviour->scripts.end(); ++it) {
				//               existed script      to add script
				if (std::strcmp( it->script_name, script_name.GetString()) == 0) break;
			}
			
			if(it == go_behaviour->scripts.end()) go_behaviour->AddScript(script_name.GetString());
		}
		if(create) go->AddComponent(go_behaviour);
		go_behaviour->LoadScripts();
	}
	}
}

Component* Factory::GetGameSideComp(StringID comp_id) {
	auto it = builder_map.find(comp_id);

	if (it != builder_map.end()) {
		auto& builder = it->second;
		return builder();
	}

	return nullptr;
}
