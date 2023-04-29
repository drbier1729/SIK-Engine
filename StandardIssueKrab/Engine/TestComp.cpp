#include "stdafx.h"

#include "GameObject.h"
#include "TestComp.h"

BEGIN_ATTRIBUTES_FOR(TestComp)
	DEFINE_MEMBER(Bool, b)
	DEFINE_MEMBER(Uint8, u8)
	DEFINE_MEMBER(Uint16, u16)
	DEFINE_MEMBER(Uint32, u32)
	DEFINE_MEMBER(Uint64, u64)
	DEFINE_MEMBER(Int8, i8)
	DEFINE_MEMBER(Int16, i16)
	DEFINE_MEMBER(Int32, i32)
	DEFINE_MEMBER(Int64, i64)
	DEFINE_MEMBER(Float32, f32)
	DEFINE_MEMBER(Float64, f64)
	DEFINE_MEMBER(Vec2, v2)
	DEFINE_MEMBER(Vec3, v3)
	DEFINE_MEMBER(Vec4, v4)
	DEFINE_MEMBER(Ivec2, iv2)
	DEFINE_MEMBER(Ivec3, iv3)
	DEFINE_MEMBER(Ivec4, iv4)
	DEFINE_MEMBER(StringID, s)
END_ATTRIBUTES


void TestComp2::OnCollide(GameObject* other) {
	SIK_INFO("Collision handled between this, {}, and other, {}", (void*)this, (void*)other);
}

BEGIN_ATTRIBUTES_FOR(TestComp2)
END_ATTRIBUTES