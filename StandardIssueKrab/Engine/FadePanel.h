#pragma once
#include "GUIObject.h"

class FadePanel : public GUIObject {
private:
	Float32 cutoff;
	Float32 ease;
public:
	FadePanel(const Ivec2& _global_space_coords, const Ivec2& _dimensions);
	~FadePanel() = default;

	void Update(Float32 dt) override;

	Float32 GetCutoff() const;
	Float32 GetEase() const;

	void SetCutoff(Float32 _cutoff);
	void SetEase(Float32 _ease);
};

