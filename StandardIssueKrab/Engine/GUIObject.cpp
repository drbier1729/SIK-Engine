#include "stdafx.h"
#include "GUIRenderer.h"
#include "GUIObject.h"
#include "Behaviour.h"
#include "ResourceManager.h"
#include "ScriptingManager.h"
#include "GUIObjectManager.h"

void GUIObject::AdjustGlobalCoords(const Ivec2& parent_coords) {
	global_space_coords += parent_coords;
}

void GUIObject::DisableRender() {
	renderer->Disable();

	for (auto& obj : embedded_objects)
		obj->DisableRender();
}

void GUIObject::EnableRender() {
	renderer->Enable();

	for (auto& obj : embedded_objects)
		obj->EnableRender();
}

GUIObject::GUIObject(const Ivec2& _global_space_coords, const Ivec2& _dimensions,
					 const Vec4& _color) :
	is_active(true), 
	global_space_coords(_global_space_coords), dimensions(_dimensions), color(_color),
	scale(Vec2(1.0f)), renderer(nullptr), 
	p_texture(nullptr), p_alt_texture(nullptr), p_behaviour(nullptr), highlight_index(-1),
	name_sid("<NO NAME>"_sid),
	type_sid()
{
}

//Base dtor
GUIObject::~GUIObject() {
	embedded_objects.clear();

	if (p_behaviour != nullptr) {
		p_scripting_manager->DeleteBehaviour(p_behaviour);
	}
}

void GUIObject::Update(Float32 dt) {
	if (!is_active)
		return;

	if (p_behaviour != nullptr) {
		p_behaviour->Update(dt);
	}

	for (auto& embedded_obj : embedded_objects) {
		embedded_obj->Update(dt);
	}
}

/*
* Set the renderer for this GUI object
* Returns : void
*/
void GUIObject::SetRenderer(GUIRenderer* _renderer) {
	renderer = _renderer;
}

/*
* Embeds GUI object within this object
* Returns : void
*/
void GUIObject::EmbedGUIObject(GUIObject* _obj) {	
	embedded_objects.push_back(_obj);
}

/*
* Returns the GUIObjects directly embedded within this object
* Returns : const Vector<GuiObject*>&
*/
const Vector<GUIObject*>& GUIObject::GetEmbeddedObjects() const {
	return embedded_objects;
}

/*
* Recursively iterates through all the embedded objects
* and returns a list of them
* Returns a vectory by copy so use sparingly.
* Returns: Vector<GUIObjects*>
*/
Vector<GUIObject*> GUIObject::GetEmbeddedObjectsRecursive() const {
	Vector<GUIObject*> ret_vec;
	Vector<GUIObject*> embedded_stack;

	for (auto& emb_obj : embedded_objects) {
		embedded_stack.push_back(emb_obj);
	}

	while (not embedded_stack.empty()) {
		GUIObject* top_obj = embedded_stack.back();
		embedded_stack.pop_back();
		for (auto& emb_obj : top_obj->embedded_objects) {
			embedded_stack.push_back(emb_obj);
		}
		ret_vec.push_back(top_obj);
	}

	return ret_vec;
}

//Checks if the game object is active
bool GUIObject::IsActive() const {
	return is_active;
}

//Disable game object
void GUIObject::Disable() {
	is_active = false;

	//This object is preset in the highlighted list.
	//Disabling should remove it from there
	if (highlight_index.x > -1) {
		p_gui_object_manager->RemoveHighlightObject(this);
	}

	if (p_behaviour != nullptr)
		p_behaviour->is_active = false;

	for (auto& obj : embedded_objects)
		obj->Disable();
}
//Enable game object
void GUIObject::Enable() {
	is_active = true;

	//This object should be preset in the highlighted list.
	//Enabling should remove it from there
	if (highlight_index.x > -1) {
		p_gui_object_manager->AddHighlightObject(this);
	}
	
	if (p_behaviour != nullptr)
		p_behaviour->is_active = true;

	for (auto& obj : embedded_objects)
		obj->Enable();
}

//Returns the local_space_coordinates
const Ivec2& GUIObject::GetGlobalSpaceCoords() const{
	return global_space_coords;
}

void GUIObject::SetGlobalSpaceCoords(const Ivec2& _coords) {
	global_space_coords = _coords;
}

//Retuns the dimensions of the object
Ivec2 GUIObject::GetDimensions() const {
	return dimensions;
}

//Sets the dimensions of the obejct
void GUIObject::SetDimensions(const Ivec2& _dimensions) {
	dimensions = _dimensions;
}

Vec2 GUIObject::GetScale() const {
	return scale;
}

float GUIObject::GetScaleX() const {
	return scale.x;
}

float GUIObject::GetScaleY() const {
	return scale.y;
}

void GUIObject::SetScale(const Vec2& _scale) {
	scale = _scale;
}

void GUIObject::SetScaleX(float _scale_x) {
	scale.x = _scale_x;
}

void GUIObject::SetScaleY(float _scale_y) {
	scale.y = _scale_y;
}

//Sets the highlight value for an object
void GUIObject::SetHighlight(Bool _val) {
	renderer->is_highlighted = _val;
}

//Gets the highlight value for an object
Bool GUIObject::GetHighlightState() const{
	return renderer->is_highlighted;
}

//Gets the color for the GUI object
Vec4 GUIObject::GetColor() const {
	return color;
}

//Sets the color for the GUI object
void GUIObject::SetColor(const Vec4& _color) {
	color = _color;
}

//Set the texture on the renderer
void GUIObject::SetTexture(Texture* _p_texture) {
	p_texture = _p_texture;
}

//Set the alternate texture on the renderer
void GUIObject::SetAltTexture(Texture* _p_texture) {
	p_alt_texture = _p_texture;
}

void GUIObject::ChangeTexture(const char* tex_name) {
	p_texture = p_resource_manager->LoadTexture(tex_name);
}

void GUIObject::ChangeAltTexture(const char* tex_name) {
	p_alt_texture = p_resource_manager->LoadTexture(tex_name);
}

Texture* GUIObject::GetTexture() const {
	return p_texture;
}

Texture* GUIObject::GetAltTexture() const {
	return p_alt_texture;
}

void GUIObject::SetAlternateRender(bool _val) {
	renderer->is_alternate = _val;
}


/*
* Check if the specified position is within the bounds of the GUI object
* Returns bool - True if overlap exists
*/
bool GUIObject::CheckPosOverlap(Ivec2 _position) {
	Ivec2 max_bounds = global_space_coords + dimensions;
	if (
		_position.x >= global_space_coords.x &&
		_position.y >= global_space_coords.y &&
		_position.x <= max_bounds.x &&
		_position.y <= max_bounds.y)
		return true;

	return false;
}

void GUIObject::SetBehaviour(Behaviour* _p_bh) {
	p_behaviour = _p_bh;
}

Behaviour* GUIObject::GetBehaviour() const {
	return p_behaviour;
}

Mat4 GUIObject::GetTransform() const {
	return glm::translate(Vec3(global_space_coords, 0.0)) * glm::scale(Vec3(scale, 1.0));
}

StringID GUIObject::GetName() const
{
	return name_sid;
}

StringID GUIObject::GetType() const
{
	return type_sid;
}

void GUIObject::SetName(StringID new_name)
{
	name_sid = new_name;
}

