#include "stdafx.h"
#include "Transform.h"

BEGIN_ATTRIBUTES_FOR(Transform)
	DEFINE_MEMBER(Vec3, position)
	DEFINE_MEMBER(Quat, orientation)
	DEFINE_MEMBER(Vec3, scale)
END_ATTRIBUTES