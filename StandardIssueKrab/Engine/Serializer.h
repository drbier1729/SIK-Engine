#pragma once

template<class C>
void DeserializeReflectable(rapidjson::Value const& json_value, C* p_comp);

template<class C>
void SerializeReflectable(rapidjson::Value& json_value, C const* p_comp, rapidjson::MemoryPoolAllocator<>& alloc);

#ifndef DEFAULT_SERIALIZE
#define DEFAULT_SERIALIZE(component)\
inline void Deserialize(rapidjson::Value const& json_value) override {\
	DeserializeReflectable<##component>(json_value, this); }\
inline void Modify(rapidjson::Value const& json_value) override {\
	DeserializeReflectable<##component>(json_value, this); }\
inline void Serialize(rapidjson::Value& json_value, rapidjson::MemoryPoolAllocator<>& alloc) override {\
	SerializeReflectable<##component>(json_value, this, alloc); }
#endif

#include "Serializer.tpp"