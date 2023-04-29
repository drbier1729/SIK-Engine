#pragma once

constexpr Uint8 MAX_FIELDS = 20;

#define ALLOW_PRIVATE_REFLECTION friend class Reflector;

class Reflector {
public:
	enum class TypeName {
		Bool,
		Uint8,
		Uint16,
		Uint32,
		Uint64,
		Int8,
		Int16,
		Int32,
		Int64,
		SizeT,
		Float32,
		Float64,
		Vec2,
		Vec3,
		Vec4,
		Ivec2,
		Ivec3,
		Ivec4,
		Quat,
		StringID
	};

	// Type information
	struct Type {
		const char* string_name;
		TypeName enum_name;
		SizeT size;
	};

	// For member variables whose metadata is accessed at runtime
	struct Field {
		Type const* type;
		const char* name;
		SizeT offset;
	};
	struct Class {
		Array<Field, MAX_FIELDS> fields;
	};

	template<typename T, Reflector::TypeName t_enum>
	static Type const* GetType();

	template<typename T>
	static Class const* GetClass();

	/*Macro Functionality:
	* ## - concatentation
	* # - stringification
	* __COUNTER__ - integral values starting from 0. resets to 0 for every new class
	* offsetof(...) - integral value in bytes from the beginning of object
	*					ex. first Int32 member in a class: returns 0
	*						second Int32 member: returns 4
	*/
	// Instantiate new types as needed
	#define DEFINE_TYPE(TYPE)\
	template<>\
	Reflector::Type const* Reflector::GetType<TYPE, Reflector::TypeName::TYPE>() {\
		static Reflector::Type type;\
		type.string_name = #TYPE;\
		type.size = sizeof(TYPE);\
		type.enum_name = Reflector::TypeName::TYPE;\
		return &type;\
	}\

	// Instantiate member data
	#define BEGIN_ATTRIBUTES_FOR(CLASS)\
	template<>\
	Reflector::Class const* Reflector::GetClass<CLASS>() {\
		using ClassType = CLASS;\
		static Reflector::Class local_class;\
		enum {\
			BASE = __COUNTER__\
		};\
		
		#define DEFINE_MEMBER(TYPE, NAME)\
		enum {\
			NAME##Index = __COUNTER__ - BASE - 1\
		};\
		local_class.fields[NAME##Index].type = Reflector::GetType<decltype(ClassType::NAME), Reflector::TypeName::TYPE>();\
		local_class.fields[NAME##Index].name = #NAME;\
		local_class.fields[NAME##Index].offset = offsetof(ClassType, NAME);\

		#define END_ATTRIBUTES\
		return &local_class;\
	}
};