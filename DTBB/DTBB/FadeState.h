#pragma once
#include "Engine/GameState.h"

class GUIObject;
class FadePanel;

class FadeState : public GameState {
public:
	FadeState(const char* fade_json, Float32 _fade_duration, 
				Bool _fade_out=true, Bool _cycle_fade=false);
	~FadeState() = default;

	void Enter() override;
	void Exit() override;

	void Update(Float32 dt) override;

	Bool IsComplete() const;

	/*
	* Resets the fade timer
	* Returns: void
	*/
	void Reset();
	void Reset(Float32 new_fade_duration);
private:
	FadePanel* p_fade;
	Float32 fade_duration;
	Float32 fade_completed;

	Int32 fade_effect_id;

	Bool fade_out;
	Bool cycle_fade;
};

