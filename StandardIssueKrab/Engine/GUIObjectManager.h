#pragma once

#include "InputAction.h"
#include "MemoryResources.h"

class GUIObject;
class GUIText;
using GUIBuilder = GUIObject * (*)(PolymorphicAllocator&, const Ivec2&, const Ivec2&);

class GUIObjectManager {
private:
	static constexpr SizeT buf_size = 64 * 1024;
	Array<Byte, buf_size> gui_obj_buffer;
	LinearMemoryResource gui_obj_mem_res{ gui_obj_buffer.data(), gui_obj_buffer.size() };
	PolymorphicAllocator gui_obj_allocator{ &gui_obj_mem_res };
	List<GUIObject*> gui_object_list;
	Vector<Vector<GUIObject*>> highlighted_matrix;
	Vector<GUIText*> text_objects;
	Int16 highlight_index_x;
	Int16 highlight_index_y;

	UnorderedMap<StringID, GUIBuilder> gui_builder_map;
	/*
	* Recursively create a hierarchy of gui objects from a json_obj
	* Returns: Vector of pointers to the created guiObjects
	*/
	Vector<GUIObject*> CreateGUI(const rapidjson::Value& json_obj);

	/*
	* Generates the GUIRenderer objects for each GUIObject
	* Returns: void
	*/
	void GenerateGUIRenderers(Vector<GUIObject*>& _created_gui_objs);

	void GenerateGUIRenderer(GUIObject* p_gui_obj);

	Vector<GUIObject*> GetOrderedRecursiveObjects(Vector<GUIObject*>& _obj_list);
public:
	GUIObjectManager();
	~GUIObjectManager();

	/*
	* Add a gui object to the list of gui objects
	* Returns: pointer to GUIObject
	*/
	GUIObject* CreateGUIObject(StringID gui_obj_type_sid,
							   const Ivec2& _local_space_coords, const Ivec2& _dimensions, 
							   const Vec2& highlight_index = Vec2(-1));

	

	/*
	* Function to create a complete GUI from a specified Json file
	* Returns: void
	*/
	Vector<GUIObject*> CreateGUIFromFile(const char* gui_file_name);

	/*
	* Calls Update() function for each gui object
	* Returns: void
	*/
	void Update(Float32 dt);

	/*
	* Checks inputs to see if there's any interaction with the UI elements
	* Returns: void
	*/
	void UpdateUIControls(Float32 dt);

	/*
	* Delete all GUI objects
	* Clears the memory resources used
	* Returns: void
	*/
	void DeleteAllGUIObjects();

	/*
	* Calls Capacity() on the underlying memory resource
	* Returns: size_t - the space left in bytes
	*/
	SizeT BufferSpaceAvailable() const;

	/*
	* Calls Size() on the underlying memory resource
	* Returns: size_t - the space left in bytes
	*/
	SizeT BufferSpaceUsed() const;

	GUIText* GetTextObject(Uint8 index);

	/*
	* Removes all highlighted objects
	* Returns: void
	*/
	void RemoveAllHighlightObjects();

	/*
	* Removes a GUIObject from the list of highltable objects
	* Returns:void
	*/
	void RemoveHighlightObject(GUIObject* obj);
	
	/*
	* Adds a GUIObject to the list of highltable objects
	* Returns:void
	*/
	void AddHighlightObject(GUIObject* obj);

	InputAction gui_action_map;
};

template<typename T>
GUIObject* BuildObject(PolymorphicAllocator& allocator, const Ivec2& _local_space_coords, 
					   const Ivec2& _dimensions) {
	return allocator.new_object<T>(_local_space_coords, _dimensions);
}

extern GUIObjectManager* p_gui_object_manager;