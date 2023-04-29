#pragma once
#include "GUIObject.h"
class Panel : public GUIObject {
public:
	Panel(const Ivec2& _global_space_coords, const Ivec2& _dimensions);
	~Panel() = default;
};

