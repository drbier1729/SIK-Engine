#include "stdafx.h"
#include "InputManager.h"
#include "InputAction.h"
#include "MemoryResources.h"
#include "GUIObject.h"
#include "Panel.h"
#include "Button.h"
#include "GUIText.h"
#include "FadePanel.h"
#include "FontTextures.h"
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "GUIObjectManager.h"
#include "ScriptingManager.h"
#include "Behaviour.h"
#include "AudioManager.h"

Uint16 MAX_HIGHLIGHT_COUNT = 16;

Vector<GUIObject*> GUIObjectManager::CreateGUI(const rapidjson::Value& json_obj) {
	rapidjson::Value::ConstMemberIterator doc_itr;

	Vector<GUIObject*> created_gui_objs;
	for (auto& gui_item : json_obj.GetObj()) {
		const rapidjson::Value& type_name = gui_item.value["Type"];
		StringID gui_obj_type = ToStringID(type_name.GetString());
		StringID gui_obj_name = ToStringID(gui_item.name.GetString());

		const rapidjson::Value& coord_array = gui_item.value["Coords"];
		SIK_ASSERT(coord_array.IsArray(), "Coordinates are not arrays");
		auto const& dimension_array =
			gui_item.value.FindMember("Dimensions")->value.GetArray();

		doc_itr = gui_item.value.FindMember("Highlighted_index");
		Vec2 highlight_indx = Vec2(-1);

		if (doc_itr != gui_item.value.MemberEnd()) {
			auto const& highlight_array = doc_itr->value.GetArray();
			highlight_indx = Vec2(highlight_array[0].GetInt(), highlight_array[1].GetInt());
		}

		//Create the GUI object with the extracted value
		GUIObject* created_obj = CreateGUIObject(
			gui_obj_type,
			Ivec2(coord_array[0].GetInt(), coord_array[1].GetInt()),
			Ivec2(dimension_array[0].GetInt(), dimension_array[1].GetInt()),
			highlight_indx);
		created_obj->SetName(gui_obj_name);

		doc_itr = gui_item.value.FindMember("Scale");
		if (doc_itr != gui_item.value.MemberEnd()) {
			auto const& scale_array =
				gui_item.value.FindMember("Scale")->value.GetArray();
			created_obj->SetScale(Vec2(scale_array[0].GetFloat(), scale_array[1].GetFloat()));
		}

		//Checking for a few optional parameters 
		doc_itr = gui_item.value.FindMember("Color");
		if (doc_itr != gui_item.value.MemberEnd()) {
			auto const& color_array = doc_itr->value.GetArray();
			created_obj->SetColor(
				Vec4(color_array[0].GetInt() / 255.0f,
					color_array[1].GetInt() / 255.0f,
					color_array[2].GetInt() / 255.0f,
					color_array[3].GetInt() / 255.0f)
			);
		}

		//Texture to be used instead of color
		doc_itr = gui_item.value.FindMember("Texture");
		if (doc_itr != gui_item.value.MemberEnd()) {
			auto tex_name = doc_itr->value.GetString();
			created_obj->SetTexture(p_resource_manager->AsyncLoadTexture(tex_name));
		}
		//Alternate textures are used for animations
		doc_itr = gui_item.value.FindMember("AltTexture");
		if (doc_itr != gui_item.value.MemberEnd()) {
			auto tex_name = doc_itr->value.GetString();
			created_obj->SetAltTexture(p_resource_manager->AsyncLoadTexture(tex_name));
		}

		//Behaviour for the UI object
		doc_itr = gui_item.value.FindMember("Behaviour");
		if (doc_itr != gui_item.value.MemberEnd()) {
			Behaviour* bh = p_scripting_manager->CreateBehaviour(created_obj);

			auto it = doc_itr->value.FindMember("scripts");
			SIK_ASSERT(it->value.IsArray(), "scripts should be in an array");
			for (auto& it2 : it->value.GetArray()) {
				SIK_ASSERT(it2.IsString(), "script filename must be a string");
				bh->AddScript(it2.GetString());
			}
			created_obj->SetBehaviour(bh);
			bh->LoadScripts();
		}

		//Checking for Text UI
		if (gui_obj_type == "TEXT"_sid) {
			auto font_tex = gui_item.value.FindMember("Font")->value.GetObj();
			auto font_tex_name  = font_tex.FindMember("ttf_name")->value.GetString();
			auto font_pixel_height = font_tex.FindMember("pixel_height")->value.GetInt();
			auto text_str = gui_item.value.FindMember("Text")->value.GetString();
			GUIText* text_obj = static_cast<GUIText*>(created_obj);
			text_obj->SetFontTextures(
				p_resource_manager->LoadFontTexture(font_tex_name, font_pixel_height));
			text_obj->SetText(text_str);
		}

		if (gui_obj_type == "BUTTON"_sid) {
			static_cast<Button*>(created_obj)->SetDefaultAction();
		}

		//Embed all the objects
		doc_itr = gui_item.value.FindMember("Embedded");
		if (doc_itr != gui_item.value.MemberEnd()) {
			Vector<GUIObject*> embedded_gui = CreateGUI(doc_itr->value.GetObj());
			for (auto e_obj : embedded_gui) {
				e_obj->AdjustGlobalCoords(Ivec2(coord_array[0].GetInt(), coord_array[1].GetInt()));
				for (auto& ee_obj : e_obj->GetEmbeddedObjects()) {
					ee_obj->AdjustGlobalCoords(Ivec2(coord_array[0].GetInt(), coord_array[1].GetInt()));
				}
				created_obj->EmbedGUIObject(e_obj);
			}
		}

		Behaviour* bh = created_obj->GetBehaviour();
		if (bh) {
			bh->LoadScripts();
		}

		created_gui_objs.push_back(created_obj);
	}

	return created_gui_objs;
}

Vector<GUIObject*> GUIObjectManager::GetOrderedRecursiveObjects(Vector<GUIObject*>& _obj_list) {
	Vector<GUIObject*> ret_list;
	std::queue<GUIObject*> gui_obj_fifo;
	
	for (auto& obj : _obj_list) {
		gui_obj_fifo.push(obj);
	}

	while (not gui_obj_fifo.empty()) {
		GUIObject* ret_obj = gui_obj_fifo.front();
		gui_obj_fifo.pop();
		for (auto& obj : ret_obj->embedded_objects) {
			gui_obj_fifo.push(obj);
		}
		ret_list.push_back(ret_obj);
	}

	return ret_list;
}

GUIObjectManager::GUIObjectManager() : highlight_index_x(0),
	highlight_index_y(0), highlighted_matrix(),
	gui_action_map("gui_action_map.json") {

	gui_builder_map.emplace(
		"PANEL"_sid,
		&(BuildObject<Panel>)
	);

	gui_builder_map.emplace(
		"BUTTON"_sid,
		&(BuildObject<Button>)
	);

	gui_builder_map.emplace(
		"TEXT"_sid,
		&(BuildObject<GUIText>)
	);

	gui_builder_map.emplace(
		"FADEPANEL"_sid,
		&(BuildObject<FadePanel>)
	);
}

GUIObjectManager::~GUIObjectManager() {
	DeleteAllGUIObjects();
}

/*
* Add a gui object to the list of gui objects
* Returns: pointer to GUIObject
*/
GUIObject* GUIObjectManager::CreateGUIObject( StringID gui_obj_type,
	const Ivec2& _local_space_coords, const Ivec2& _dimensions, 
	const Vec2& highlight_index) {
	GUIObject* new_obj =
		gui_builder_map[gui_obj_type](gui_obj_allocator, _local_space_coords, _dimensions);
	if (highlight_index.x != -1) {
		new_obj->highlight_index = highlight_index;
		AddHighlightObject(new_obj);
	}
	GUIText* text_obj = dynamic_cast<GUIText*>(new_obj);
	if (text_obj)
		text_objects.push_back(text_obj);

	gui_object_list.push_back(new_obj);

	return new_obj;
}

/*
* Generates the GUIRenderer objects for each GUIObject
* Returns: void 
*/
void GUIObjectManager::GenerateGUIRenderers(Vector<GUIObject*>& _created_gui_objs) {
	Vector<GUIObject*> created_objs_recursive = GetOrderedRecursiveObjects(_created_gui_objs);
	for (const auto& gui_obj : created_objs_recursive) {
		//Create the corresponding renderer for the gui object
		GUIRenderer* new_renderer = p_graphics_manager->CreateGUIRenderer(gui_obj);
		gui_obj->SetRenderer(new_renderer);
	}
}

/*
* Generates a GUIRenderer object for the gui_object specified in the argument
* Returns: void
*/
void GUIObjectManager::GenerateGUIRenderer(GUIObject* p_gui_obj) {
	p_gui_obj->SetRenderer(
		p_graphics_manager->CreateGUIRenderer(p_gui_obj)
	);
}

/*
* Function to create a complete GUI from a specified Json file
* Returns: void
*/
Vector<GUIObject*> GUIObjectManager::CreateGUIFromFile(const char* gui_file_name) {
	JSON *doc{ p_resource_manager->LoadJSON(gui_file_name) };
	Vector<GUIObject*> created_objs = CreateGUI(doc->doc.GetObj());
	
	GenerateGUIRenderers(created_objs);
	return created_objs;
}


/*
* Calls Update() function for each gui object
* Returns: void
*/
void GUIObjectManager::Update(Float32 dt) {
	for (auto gui_object : gui_object_list) {
		gui_object->Update(dt);
	}
}

void GUIObjectManager::UpdateUIControls(Float32 dt) {
	if (highlighted_matrix.size() == 0)
		return;

	GUIObject* p_gui_obj;

	Int32 new_highlight_index_x = highlight_index_x;
	Int32 new_highlight_index_y = highlight_index_y;

	//Check if the mouse was moved last frame
	if (p_input_manager->GetMouseDelta() != Ivec2(0, 0)) {
		Ivec2 curr_mouse_pos = p_input_manager->GetMousePos();
		//Check all the objects that can be highlighted
		for (Uint16 indx_x = 0; indx_x < highlighted_matrix.size(); ++indx_x) {
			for (Uint16 indx_y = 0; indx_y < highlighted_matrix[indx_x].size(); ++indx_y) {
				p_gui_obj = highlighted_matrix[indx_x][indx_y];
				if (p_gui_obj == nullptr)
					continue;
				if (not p_gui_obj->is_active)
					continue;
				//If the mouse position is currently over an object
				if (p_gui_obj->CheckPosOverlap(curr_mouse_pos)) {

					new_highlight_index_x = indx_x;
					new_highlight_index_y = indx_y;
				}
			}
		}
	}

	//Move the highlight up if the UP action was pressed
	if (gui_action_map.IsActionTriggered(InputAction::Actions::UP) ||
		gui_action_map.IsActionTriggered(InputAction::Actions::UP_ALT))
		new_highlight_index_x--;
		
	//Move the highlight down if the DOWN action was pressed
	if (gui_action_map.IsActionTriggered(InputAction::Actions::DOWN) ||
		gui_action_map.IsActionTriggered(InputAction::Actions::DOWN_ALT))
		new_highlight_index_x++;


	new_highlight_index_x = (new_highlight_index_x + highlighted_matrix.size()) % highlighted_matrix.size();

	//Move the highlight left if the LEFT action was pressed
	if (gui_action_map.IsActionTriggered(InputAction::Actions::LEFT) ||
		gui_action_map.IsActionTriggered(InputAction::Actions::LEFT_ALT))
		new_highlight_index_y--;

	//Move the highlight right if the RIGHT action was pressed
	if (gui_action_map.IsActionTriggered(InputAction::Actions::RIGHT) ||
		gui_action_map.IsActionTriggered(InputAction::Actions::RIGHT_ALT))
		new_highlight_index_y++;

	new_highlight_index_y = (new_highlight_index_y + highlighted_matrix[new_highlight_index_x].size()) % highlighted_matrix[new_highlight_index_x].size();

	//Set the object to be highlighted for rendering 
	for (auto& highlight_list : highlighted_matrix) {
		for (auto& highlight_obj : highlight_list) {
			highlight_obj->SetHighlight(false);
		}
	}

	// Play highlight SFX
	if (highlight_index_x != new_highlight_index_x || highlight_index_y != new_highlight_index_y) {
		p_audio_manager->PlayAudio("BUTTON_HOVER"_sid,
			p_audio_manager->sfx_chanel_group,
			100.0f,
			1.0f,
			false,
			0);
	}
	
	//mark it as highlighted
	highlight_index_x = new_highlight_index_x;
	highlight_index_y = new_highlight_index_y;
	highlighted_matrix[highlight_index_x][highlight_index_y]->SetHighlight(true);
}

/*
* Delete all GUI objects
* Clears the memory resources used
* Returns: void
*/
void GUIObjectManager::DeleteAllGUIObjects(){
	p_graphics_manager->DeleteAllGUIRenderers();
	highlight_index_x = 0;
	highlight_index_y = 0;
	highlighted_matrix.clear();
	for (auto game_object : gui_object_list) {
		gui_obj_allocator.delete_object(game_object);
	}
	gui_obj_mem_res.Clear();
	gui_object_list.clear();
	text_objects.clear();
}

/*
* Calls Capacity() on the underlying memory resource
* Returns: size_t - the space left in bytes
*/
SizeT GUIObjectManager::BufferSpaceAvailable() const {
	return gui_obj_mem_res.Capacity();
}

/*
* Calls Size() on the underlying memory resource
* Returns: size_t - the space left in bytes
*/
SizeT GUIObjectManager::BufferSpaceUsed() const {
	return gui_obj_mem_res.Size();
}

GUIText* GUIObjectManager::GetTextObject(Uint8 index) {
	return text_objects[index];
}

void GUIObjectManager::RemoveAllHighlightObjects() {
	//Reset the highlight index in case of any changes
	highlight_index_x = 0;
	highlight_index_y = 0;

	highlighted_matrix.clear();
}

void GUIObjectManager::RemoveHighlightObject(GUIObject* obj) {
	//Reset the highlight index in case of any changes
	highlight_index_x = 0;
	highlight_index_y = 0;

	Ivec2 obj_highlight_indx = obj->highlight_index;

	if (highlighted_matrix.size() <= obj_highlight_indx.y) {
		SIK_WARN("Removing obj from an empty slot");
		return;
	}
		
	auto start_iter = highlighted_matrix[obj_highlight_indx.y].begin();

	if (highlighted_matrix[obj_highlight_indx.y].size() <= obj_highlight_indx.x) {
		SIK_WARN("Removing obj from an empty slot");
		return;
	}

	highlighted_matrix[obj_highlight_indx.y].erase(start_iter + obj_highlight_indx.x);
}

void GUIObjectManager::AddHighlightObject(GUIObject* obj) {
	//Reset the highlight index in case of any changes
	highlight_index_x = 0;
	highlight_index_y = 0;

	Ivec2 obj_highlight_indx = obj->highlight_index;
	Vector<GUIObject*> empty_list;

	while (highlighted_matrix.size() <= obj_highlight_indx.x) {
		highlighted_matrix.push_back(empty_list);
	}

	while (highlighted_matrix[obj_highlight_indx.x].size() <= obj_highlight_indx.y) {
		highlighted_matrix[obj_highlight_indx.x].push_back(nullptr);
	}

	if (highlighted_matrix[obj_highlight_indx.x][obj_highlight_indx.y] != nullptr) {
		SIK_WARN("Object already in highlight list at index y \"{}\"", obj_highlight_indx.y);
	}

	highlighted_matrix[obj_highlight_indx.x][obj_highlight_indx.y] = obj;
}