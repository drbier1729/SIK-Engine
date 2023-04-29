#pragma once

#include "Serializer.h"
#include "Component.h"

class TestComp : public Component {
public:
	VALID_COMPONENT(TestComp);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(TestComp);

private:
	Bool b;
	Uint8 u8;
	Uint16 u16;
	Uint32 u32;
	Uint64 u64;
	Int8 i8;
	Int16 i16;
	Int32 i32;
	Int64 i64;
	Float32 f32;
	Float64 f64;
	Vec2 v2;
	Vec3 v3;
	Vec4 v4;
	Ivec2 iv2;
	Ivec3 iv3;
	Ivec4 iv4;
	StringID s;
};

class TestComp2 : public Component {
public:
	VALID_COMPONENT(TestComp2);
	ALLOW_PRIVATE_REFLECTION;
	DEFAULT_SERIALIZE(TestComp2);

	void OnCollide(GameObject* other) override;

private:
	Vector<Uint32> vec;
};