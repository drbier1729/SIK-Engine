#pragma once

#include "GameObject.h"

class WorldEditor 
{
private:    
    
    rapidjson::Document SceneDocument;
    String open_filename;
    UnorderedMap<const char*, Vector<std::pair<StringID, Bool>>> toEditList;

    Vector<std::pair<GameObject*, StringID>> saveGameObjectList;
 

public:
    WorldEditor() {  open_filename = "TestScene.json"; };
    ~WorldEditor() = default;

    void Init();
    void Destroy();
     
    void SetOpenFileName(const char* filename) { open_filename = String(filename); }
    const char* GetSceneFileName() { return open_filename.c_str(); }

    // Save scene to JSON file
    void SaveScene();
    
    // modify components in scene file
    void AddComp(GameObject const* go, StringID const& comp_name, Bool const& add = true);
    void RemoveComp(GameObject const* go, StringID const& comp_name);

    void SaveNewGameObject(GameObject* go, StringID const& archetype);
};

extern WorldEditor* p_world_editor;