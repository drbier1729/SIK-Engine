template<class C>
void DeserializeReflectable(rapidjson::Value const& json_value, C* p_comp) {
	Reflector::Class const* object_info = Reflector::GetClass<C>();

	for (auto const& field : object_info->fields) {
		if (field.type == nullptr) {
			break;
		}

		if (json_value.HasMember(field.name)) {
			Byte* destination = reinterpret_cast<Byte*>(p_comp) + field.offset;
			switch (field.type->enum_name) {
			case Reflector::TypeName::Bool: {
				Bool source = json_value[field.name].GetBool();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Uint8:
			case Reflector::TypeName::Uint16:
			case Reflector::TypeName::Uint32: {
				Uint32 source = json_value[field.name].GetUint();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Uint64: {
				Uint64 source = json_value[field.name].GetUint64();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Int8:
			case Reflector::TypeName::Int16:
			case Reflector::TypeName::Int32: {
				Int32 source = json_value[field.name].GetInt();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Int64: {
				Int64 source = json_value[field.name].GetInt64();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Float32: {
				Float32 source = json_value[field.name].GetFloat();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Float64: {
				Float64 source = json_value[field.name].GetDouble();
				memcpy(destination, &source, field.type->size);
				break;
			}
			case Reflector::TypeName::Vec2:
			case Reflector::TypeName::Vec3:
			case Reflector::TypeName::Vec4: {
				SIK_ASSERT(json_value[field.name].IsArray(), "JSON value is not an array.");
				auto const& source = json_value[field.name].GetArray();
				SIK_ASSERT(source.Size() == (field.type->size / sizeof(Float32)), "JSON array length does not match vector length.");
				for (rapidjson::SizeType i = 0; i < source.Size(); ++i) {
					Float32 val = source[i].GetFloat();
					memcpy(destination + (i * sizeof(Float32)), &val, sizeof(Float32));
				}
				break;
			}
			case Reflector::TypeName::Ivec2:
			case Reflector::TypeName::Ivec3:
			case Reflector::TypeName::Ivec4: {
				SIK_ASSERT(json_value[field.name].IsArray(), "JSON value is not an array.");
				auto const& source = json_value[field.name].GetArray();
				SIK_ASSERT(source.Size() == (field.type->size / sizeof(Int32)), "JSON array length does not match vector length.");
				for (rapidjson::SizeType i = 0; i < source.Size(); ++i) {
					Int32 val = source[i].GetInt();
					memcpy(destination + (i * sizeof(Int32)), &val, sizeof(Int32));
				}
				break;
			}
			case Reflector::TypeName::Quat: {
				// Deserializes from Euler Angles
				SIK_ASSERT(json_value[field.name].IsArray(), "JSON value is not an array.");
				auto const& source = json_value[field.name].GetArray();
				SIK_ASSERT(source.Size() == 3, "JSON array length does not match vector length (3) for Euler Angles.");

				Quat quat = glm::rotate(Quat(1, 0, 0, 0), glm::radians(source[0].GetFloat()), Vec3(1, 0, 0)) *
					        glm::rotate(Quat(1, 0, 0, 0), glm::radians(source[1].GetFloat()), Vec3(0, 1, 0)) *
					        glm::rotate(Quat(1, 0, 0, 0), glm::radians(source[2].GetFloat()), Vec3(0, 0, 1));
				
				memcpy(destination, &quat, sizeof(Quat));
				break;
			}
			case Reflector::TypeName::StringID: {
				rapidjson::Value const& sid = json_value[field.name];
				StringID source = sid.IsString() ? ToStringID(sid.GetString()) : StringID{ sid.GetUint64() };
				memcpy(destination, &source, field.type->size);
				break;
			}
			default: {
				SIK_ASSERT(false, "Non reflectable type deserialized.");
				break;
			}
			}
		}
	}
}

template<class C>
void SerializeReflectable(rapidjson::Value& json_value, C const* p_comp, rapidjson::MemoryPoolAllocator<>& alloc) {
	Reflector::Class const* object_info = Reflector::GetClass<C>();

	for (auto const& field : object_info->fields) {
		if (field.type == nullptr) {
			break;
		}

		// can't find member, add corresponding member 
		if (json_value.FindMember(field.name) == json_value.MemberEnd()) {
			switch (field.type->enum_name) {
			case Reflector::TypeName::Bool: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kTrueType), alloc);
				break;
			}
			case Reflector::TypeName::Uint8:
			case Reflector::TypeName::Uint16:
			case Reflector::TypeName::Uint32:
			case Reflector::TypeName::Uint64: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kNumberType), alloc);
				break;
			}
			case Reflector::TypeName::Int8:
			case Reflector::TypeName::Int16:
			case Reflector::TypeName::Int32:
			case Reflector::TypeName::Int64: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kNumberType), alloc);
				break;
			}
			case Reflector::TypeName::Float32:
			case Reflector::TypeName::Float64: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kNumberType), alloc);
				break;
			}
			case Reflector::TypeName::Vec2:
			case Reflector::TypeName::Vec3:
			case Reflector::TypeName::Vec4: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kArrayType), alloc);
				break;
			}
			case Reflector::TypeName::Ivec2:
			case Reflector::TypeName::Ivec3:
			case Reflector::TypeName::Ivec4: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kArrayType), alloc);
				break;
			}
			case Reflector::TypeName::Quat: {
				json_value.AddMember(rapidjson::StringRef<char>(field.name), rapidjson::Value(rapidjson::kArrayType), alloc);
				break;
			}
			}
		}
		
		Byte const* source = reinterpret_cast<Byte const*>(p_comp) + field.offset;

		switch (field.type->enum_name) {
		case Reflector::TypeName::Bool: {
			Bool destination = false;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetBool(destination);
			break;
		}
		case Reflector::TypeName::Uint8:
		case Reflector::TypeName::Uint16:
		case Reflector::TypeName::Uint32: {
			Uint32 destination = 0;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetUint(destination);
			break;
		}
		case Reflector::TypeName::Uint64: {
			Uint64 destination = 0;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetUint64(destination);
			break;
		}
		case Reflector::TypeName::Int8:
		case Reflector::TypeName::Int16:
		case Reflector::TypeName::Int32: {
			Int32 destination = 0;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetUint(destination);
			break;
		}
		case Reflector::TypeName::Int64: {
			Int64 destination = 0;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetUint64(destination);
			break;
		}
		case Reflector::TypeName::Float32: {
			Float32 destination = 0.0f;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetFloat(destination);
			break;
		}
		case Reflector::TypeName::Float64: {
			Float64 destination = 0.0;
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetDouble(destination);
			break;
		}
		case Reflector::TypeName::Vec2: {
			Vec2 destination = Vec2{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(2, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc);
			break;
		}
		case Reflector::TypeName::Vec3: {
			Vec3 destination = Vec3{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(3, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc)
				.PushBack(destination.z, alloc);
			break;
		}
		case Reflector::TypeName::Vec4: {
			Vec4 destination = Vec4{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(4, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc)
				.PushBack(destination.z, alloc)
				.PushBack(destination.w, alloc);
			break;
		}
		case Reflector::TypeName::Ivec2: {
			Ivec2 destination = Ivec2{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(2, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc);
			break;
		}
		case Reflector::TypeName::Ivec3: {
			Ivec3 destination = Ivec3{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(3, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc)
				.PushBack(destination.z, alloc);
			break;
		}
		case Reflector::TypeName::Ivec4: {
			Ivec4 destination = Ivec4{ 0 };
			memcpy(&destination, source, field.type->size);
			json_value[field.name].SetArray()
				.Reserve(4, alloc)
				.PushBack(destination.x, alloc)
				.PushBack(destination.y, alloc)
				.PushBack(destination.z, alloc)
				.PushBack(destination.w, alloc);
			break;
		}
		case Reflector::TypeName::StringID: {
			StringID destination;
			memcpy(&destination, source, field.type->size);
#ifdef STR_DEBUG
			//json_value[field.name].SetString(rapidjson::StringRef(DebugStringLookUp(destination)));
			json_value[field.name].SetUint64(static_cast<Uint64>(destination));
#else
			json_value[field.name].SetUint64(static_cast<Uint64>(destination));
#endif
			break;
		}
		case Reflector::TypeName::Quat: {
			Quat destination = Quat(1, 0, 0, 0);

			memcpy(&destination, source, field.type->size);
			Vec3 euler = glm::eulerAngles(destination) * 180.0f / 3.14159f;
			json_value[field.name].SetArray()
				.Reserve(3, alloc)
				.PushBack(euler.x, alloc)
				.PushBack(euler.y, alloc)
				.PushBack(euler.z, alloc);		
			break;
		}
		default: {
			SIK_ASSERT(false, "Non reflectable type serialized");
			break;
		}
		}
	}
}