#pragma once
#include "InputAction.h"

class GUIRenderer;
class Texture;
class Behaviour;

class GUIObject {
private:
	Bool is_active;
	Ivec2 global_space_coords;
	Ivec2 dimensions;
	Vec2 scale;
	Vec4 color;
	Ivec2 highlight_index;
	Texture* p_texture;
	Texture* p_alt_texture;
	GUIRenderer* renderer;
	Vector<GUIObject*> embedded_objects;
	StringID name_sid;
	StringID type_sid;
protected:
	Behaviour* p_behaviour;
public:
	GUIObject(const Ivec2& _global_space_coords, const Ivec2& _dimensions, 
			  const Vec4& _color = Vec4(0.0f));
	virtual ~GUIObject();
	virtual void Update(Float32 dt);

	/*
	* Set the renderer for this GUI object
	* Returns : void
	*/
	void SetRenderer(GUIRenderer* _renderer);
	/*
	* Embeds GUI object within this object
	* Returns : void
	*/
	void EmbedGUIObject(GUIObject* _obj);

	/*
	* Returns the GUIObjects directly embedded within this object
	* Returns : const Vector<GuiObject*>&
	*/
	const Vector<GUIObject*>& GetEmbeddedObjects() const;

	/*
	* Recursively iterates through all the embedded objects 
	* and returns a list of them
	* Returns a vectory by copy so use sparingly.
	* Returns: Vector<GUIObjects*>
	*/
	Vector<GUIObject*> GetEmbeddedObjectsRecursive() const;

	//Checks if the game object is active
	bool IsActive() const;

	//Disable game object
	void Disable();

	//Enable game object
	void Enable();

	//Returns the global_space_coordinates
	const Ivec2& GetGlobalSpaceCoords() const;

	//Sets the global_space_coords
	void SetGlobalSpaceCoords(const Ivec2& _coords);

	//Retuns the dimensions of the object
	Ivec2 GetDimensions() const;

	//Sets the dimensions of the obejct
	void SetDimensions(const Ivec2& _dimensions);

	Vec2 GetScale() const;
	float GetScaleX() const;
	float GetScaleY() const;

	void SetScale(const Vec2& _scale);

	void SetScaleX(float _scale_x);

	void SetScaleY(float _scale_y);

	//Sets the highlight value for an object
	void SetHighlight(Bool _val);

	Bool GetHighlightState() const;

	//Gets the color for the GUI object
	Vec4 GetColor() const;

	//Sets the color for the GUI object
	void SetColor(const Vec4& _color);

	//Set the texture on the renderer
	void SetTexture(class Texture* _p_texture);

	//Set the alternate texture
	void SetAltTexture(Texture* _p_texture);

	void ChangeTexture(const char* tex_name);

	void ChangeAltTexture(const char* tex_name);

	//Get the texture
	Texture* GetTexture() const;

	//Get the alternate texture
	Texture* GetAltTexture() const;

	//Sets the alternate render for the renderer object
	void SetAlternateRender(bool _val);

	/*
	* Check if the specified position is within the bounds of the GUI object
	* Returns bool - True if overlap exists
	*/
	bool CheckPosOverlap(Ivec2 _position);

	void SetBehaviour(Behaviour* _p_bh);
	Behaviour* GetBehaviour() const;

	/*
	* Get the transform matrix for rendering
	* Returns: Mat4 - The translate * scale matrix
	*/
	Mat4 GetTransform() const;

	/*
	* Function to adjust the global coords according to
	* the parent objects coordinates
	*/
	void AdjustGlobalCoords(const Ivec2& parent_coords);

	Int16 GetHighlightIndex() const;

	/*
	* Only disables the rendering but doesn't affect the Update
	* Returns: void
	*/
	void DisableRender();

	/*
	* Only enables the rendering but doesn't affect the Update
	* Returns: void
	*/
	void EnableRender();

	void SetName(StringID new_name);
	StringID GetName() const;
	StringID GetType() const;

	friend class GUIObjectManager;
};

