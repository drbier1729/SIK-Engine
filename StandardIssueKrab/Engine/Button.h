#pragma once
#include "GUIObject.h"

//Callback type that takes no arguments and returns void
typedef void (*ButtonAction)();

class Behaviour;

class Button : public GUIObject {
private:
	ButtonAction action;

	//Determines whether you can click the button
	Bool is_clickable;

	/*
	* Determines whether the button is currenly pressed down
	* Can only perform the action when this state is true
	*/
	Bool is_clicked;

	/*
	* Determines when to perform the button button action
	*/
	Bool is_clicked_and_released;

public:
	//Ctor for button
	Button(const Vec2& _global_space_coords, const Vec2& _dimensions);
	//Dtor for button
	~Button();

	void Update(Float32 dt) override;

	/*
	* Returns the clickable state of the button
	* Return: bool - True if clickable
	*/
	Bool GetClickableState() const;

	//Sets the clickable state to false
	void DisableClickable();

	//Sets the clickable state to true
	void EnableClickable();

	/*
	* Sets a callback as the action to perform when the button is clicked
	* Returns: void
	*/
	void SetAction(ButtonAction _action);

	void SetDefaultAction();

	/*
	* Sets the state of the button in the script to check for actions
	* Returns: void
	*/
	void SetScriptState();

	/*
	* Play the click sound.
	* Returns: void
	*/
	void PlayClickSound();


	/*
	* Play the disabled click sound.
	* Returns: void
	*/
	void PlayDisabledClickSound();
};

