#include "stdafx.h"
#include "MemoryManager.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "ScriptingManager.h"
#include "Transform.h"
#include "RigidBody.h"
#include "Serializer.h"

#include "WorldEditor.h"

void WorldEditor::Init()
{
    
}

void WorldEditor::Destroy()
{

}

template <class C>
void SaveComponent(C* p_comp, rapidjson::Value &value, rapidjson::MemoryPoolAllocator<>& alloc) {
    
    // null check
    if (p_comp == nullptr) return;

    Reflector::Class const* object_info = Reflector::GetClass<C>();
    for (auto const& field : object_info->fields) {
        if (field.type == nullptr) {
            break;
        }

        Byte* source = reinterpret_cast<Byte*>(p_comp) + field.offset;
        switch (field.type->enum_name) {
        case Reflector::TypeName::Vec3:
        case Reflector::TypeName::Vec4: {
            if (value.FindMember(field.name) == value.MemberEnd())
            value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kArrayType), alloc);
            break;
        }
        case Reflector::TypeName::Quat: { break; }
        }
    }
    
}

void WorldEditor::SaveScene()
{
    rapidjson::Document doc;

    FILE* file;
    errno_t err;

    // TODO: filepath should be read from resource manager
    err = fopen_s(&file, open_filename.c_str(), "rb");
    SIK_ASSERT(!err, "File open failed\n");

    // static_cast triggers error
    char* readBuffer = reinterpret_cast<char*>(p_memory_manager->GetCurrentAllocator().allocate(65536));
    rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

    doc.ParseStream(is);    
    SIK_ASSERT(doc.IsObject(), "JSON Format Error");

    fclose(file);
    p_memory_manager->GetCurrentAllocator().deallocate(reinterpret_cast<Byte*>(readBuffer), 65536);

    rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

#ifdef STR_DEBUG

    // add new objects before save components
    for (auto new_go : saveGameObjectList) {
        const char* obj_name = new_go.first->GetName().c_str();
        const char* archetype = DebugStringLookUp(new_go.second);

        // json objects for saving
        rapidjson::Value v(rapidjson::kObjectType);
        rapidjson::Value name_str(rapidjson::kStringType);
        rapidjson::Value archetype_str(rapidjson::kStringType);
        rapidjson::Value component_obj(rapidjson::kObjectType);

        name_str.SetString(obj_name,       alloc);
        archetype_str.SetString(archetype, alloc);



        // create and fill in RigidBody for new objects
        auto rb = new_go.first->HasComponent<RigidBody>();
        if (rb != nullptr && !rb->IsStatic()) {
            rapidjson::Value rigidbody_obj(rapidjson::kObjectType);
            rapidjson::Value rb_position_array(rapidjson::kArrayType);
            
            rb_position_array.PushBack(rb->position.x, alloc);
            rb_position_array.PushBack(rb->position.y, alloc);
            rb_position_array.PushBack(rb->position.z, alloc);
            
            rigidbody_obj.AddMember("position", rb_position_array, alloc);
            component_obj.AddMember("RigidBody", rigidbody_obj, alloc);
        }
        else {
            // create transform for new object to store position
            rapidjson::Value transform_obj(rapidjson::kObjectType);
            component_obj.AddMember("Transform", transform_obj, alloc);
        }


        // create Components for new object
        v.AddMember("Name",       name_str,      alloc);
        v.AddMember("Archetype",  archetype_str, alloc);
        v.AddMember("Components", component_obj, alloc);

        doc.AddMember(rapidjson::StringRef<char>(obj_name), v, alloc);
    }

    // clear after all new objects are saved
    saveGameObjectList.clear();

#endif


    auto* pool = &p_game_obj_manager->GetGameObjectContainer();    
    // for objects in the scene
    for (auto r = pool->all(); not r.is_empty(); r.pop_front()) {
        const char* obj_name = r.front().GetName().c_str();
        
        // find object in json
        for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
            rapidjson::GenericObject obj = it->value.GetObj();

            const char* obj_name_json = obj.FindMember("Name") == obj.MemberEnd() ? it->name.GetString() : obj["Name"].GetString();
            
            // object already existed, override & create components
            if (strcmp(obj_name, obj_name_json) == 0) {
                
                GameObject *game_obj = &r.front();
                auto components = obj.FindMember("Components");
                if (components == obj.MemberEnd()) {
                    continue;
                    rapidjson::Value v(rapidjson::kObjectType);
                    obj.AddMember("Components", v, alloc);
                }

                // Transform : Save transform whenever transform field is found, otherwise, do nothing 
                // RigidBody : Save rigidbody whenever rigidbody field is found, can be deleted
                // Behaviour : Save behaviour no matter behaviour field exists , can be deleted
                // Others    : Add / remove depends on add / remove button being pressed

                
                // Transform
                // Always save transform if there is 
                Transform* transform = game_obj->HasComponent<Transform>();
                if (transform != nullptr) {
                    auto tr = components->value.FindMember("Transform");
                    if (tr != components->value.MemberEnd()) {
                        SaveComponent(transform, tr->value, alloc);
                        SerializeReflectable(tr->value, transform, alloc);
                    }
                }

                // Behaviour
                Behaviour* behaviour = game_obj->HasComponent<Behaviour>();
                

                auto bh = components->value.FindMember("Behaviour");
                    
                auto it = toEditList.find(obj_name);
                Bool add = true;
                if (it != toEditList.end())
                for (auto comp : it->second) {
                    if (comp.first == "Behaviour"_sid) {
                        add = comp.second;
                        break;
                    }
                }
                    
                // remove
                if (!add) {
                    components->value.RemoveMember("Behaviour");
                }
                // add
                else {
                    if (behaviour != nullptr) {

                        // create if not found
                        if (bh == components->value.MemberEnd()) {
                            components->value.AddMember("Behaviour", rapidjson::kObjectType, alloc);
                            bh = components->value.FindMember("Behaviour");
                        }

                        if (bh != components->value.MemberEnd()) {

                            // create member
                            if (bh->value.FindMember("scripts") == bh->value.MemberEnd())
                                bh->value.AddMember("scripts", rapidjson::Value(rapidjson::kArrayType), alloc);

                            // get destination
                            auto scripts = bh->value["scripts"].GetArray();
                            scripts.Clear();

                            // loop source and save to destination
                            for (auto && script : behaviour->scripts) {
                                scripts.PushBack(rapidjson::StringRef<char>(script.script_name), alloc);
                            }
                        }
                    }
                }
                

                for (auto comp = game_obj->GetComponentArray().begin(); comp != game_obj->GetComponentArray().end(); ++comp) {
                    Component* p_comp = comp->get();

                    const char* comp_name = p_comp->GetName();

                    // check if it needs to add or remove
                    auto it = toEditList.find(obj_name);
                    if (it != toEditList.end()) {
                        for (auto item : it->second) {

                            Bool add = item.second;
                            
                            // add new comp
                            if (add) {
#ifdef STR_DEBUG
                                // check if the field exists first
                                if(components->value.FindMember(rapidjson::StringRef<char>(DebugStringLookUp(item.first))) == components->value.MemberEnd())
                                components->value.AddMember(rapidjson::StringRef<char>(DebugStringLookUp(item.first)), rapidjson::Value(rapidjson::kObjectType), alloc);
#endif
                            }
                            // remove member
                            else {  
                                components->value.RemoveMember(comp_name);
                            }
                        }
                    }

                    auto loc = components->value.FindMember(comp_name);
                    if(loc != components->value.MemberEnd())
                    p_comp->Serialize(loc->value, alloc);
                }
                break;
            }
            else {
                // object not found
            }
        }

    }

    err = fopen_s(&file, open_filename.c_str(), "wb");
    SIK_ASSERT(!err, "File open failed\n");

    char* writeBuffer = reinterpret_cast<char*>(p_memory_manager->GetCurrentAllocator().allocate(65536));
    rapidjson::FileWriteStream os(file, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);

    fclose(file);
    p_memory_manager->GetCurrentAllocator().deallocate(reinterpret_cast<Byte*>(writeBuffer), 65536);

    p_resource_manager->ReloadJSON(open_filename.c_str());

    // complete saving, clear the list
    toEditList.clear();
}

void WorldEditor::AddComp(GameObject const* go, StringID const& comp_name, Bool const& add) {

    // null check
    if (go == nullptr) return;

    // get key
    const char* go_name = go->GetName().c_str();
    
    // search
    auto it = toEditList.find(go_name);
    
    // create if not found
    if (it == toEditList.end()) {
        Vector<std::pair<StringID, Bool>> vec = {std::make_pair(comp_name, add)};
        toEditList.insert({ go_name, vec });
    }
    else {
        for (auto comp : it->second) {
            // first  : component name
            // second : add / remov
            // override previous action
            if (comp.first == comp_name) {
                comp.second = add;
                return;
            }
        }

        // push new one if not found
        it->second.push_back(std::make_pair(comp_name, add));
    }
}

void WorldEditor::RemoveComp(GameObject const* go, StringID const& comp_name) {
    AddComp(go, comp_name, false);
}

void WorldEditor::SaveNewGameObject(GameObject* go, StringID const& archetype) {
    saveGameObjectList.push_back(std::make_pair(go, archetype));
}
