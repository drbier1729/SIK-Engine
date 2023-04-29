#pragma once

////////////////////////////////////////////////////////////////////////////////
// Required External Includes To Use SIK Types
////////////////////////////////////////////////////////////////////////////////

// C Std
#include <cstddef>
#include <cstdint>

// Containers
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <string>
#include <bitset>
#include <tuple>
#include <queue>
#include <variant>

// Files
#include <filesystem>
#include <fstream>

// Multithreading
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

// Memory
#include <memory>
#include <memory_resource>

// Utilities
#include <chrono>
using namespace std::chrono_literals;

// Math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>

////////////////////////////////////////////////////////////////////////////////
// Type Defs
////////////////////////////////////////////////////////////////////////////////

using Bool = bool;

using Byte = std::byte;

using Uint8  = std::uint8_t;
using Uint16 = std::uint16_t;
using Uint32 = std::uint32_t;
using Uint64 = std::uint64_t;

using Int8  = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;

using SizeT = std::size_t;
 
using Float32 = float;
using Float64 = double;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Ivec2 = glm::ivec2;
using Ivec3 = glm::ivec3;
using Ivec4 = glm::ivec4;
using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;
using Quat = glm::quat;

template<class T, size_t N>
using Array = std::array<T, N>;

template<class T>
using Vector = std::pmr::vector<T>;

template<class Key, class T, class Compare = std::less<Key>>
using Map = std::pmr::map<Key, T, Compare>;

template<class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
using UnorderedMap = std::pmr::unordered_map<Key, T, Hash, Pred>;

template<class Key, class Compare = std::less<Key>>
using Set = std::pmr::set<Key, Compare>;

template<class Key, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
using UnorderedSet = std::pmr::unordered_set<Key, Hash, Pred>;

template<class T>
using List = std::pmr::list<T>;

template<std::size_t N>
using Bitset = std::bitset<N>;

template<class ... Ts>
using Tuple = std::tuple<Ts...>;

template<class CharT>
using BasicString = std::pmr::basic_string<CharT>;
using String = std::pmr::string;
using WString = std::pmr::wstring;
using U8String = std::pmr::u8string;
using U16String = std::pmr::u16string;
using U32String = std::pmr::u32string;


template<class T, class Deleter = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Deleter>;

using PolymorphicAllocator = std::pmr::polymorphic_allocator<>;
using MemoryResource = std::pmr::memory_resource;
