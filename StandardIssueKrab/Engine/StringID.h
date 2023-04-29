#pragma once
/*
* Utilities for handling string literals within runtime code. It is often useful to use
* strings as identifiers throughout the codebase, however the runtime expense of parsing
* strings is high and unnecessary. Instead, we hash our string literals at compile-time
* and store them as 64-bit StringID enums. This allows us to do things like:
* 
*   // definition
*	void foo(StringID str) {
*		switch(str)
*		{
*			break; case "bar"_sid: { 
*				// do something bar-related 
*			}
*			break; case "baz"_sid: { 
*				// do something baz-related 
*			}
*			break; default: {}
*		}
*   }
* 
*   // call
*	foo("bar"_sid);
*/


// String Hashing
namespace detail {
	inline constexpr Uint64 val_64_const = 0xcbf29ce484222325;
	inline constexpr Uint64 prime_64_const = 0x100000001b3;

	constexpr Uint64 fnv1a_64(const char* const str, const Uint64 value = val_64_const) noexcept {
		return (str[0] == '\0') ? value : fnv1a_64(&str[1], (value ^ Uint64(str[0])) * prime_64_const);
	}
	
	constexpr Uint64 fnv1a_n_64(const char* const str, std::size_t len, const Uint64 value = val_64_const) noexcept {
		Uint64 result = value;
		for(std::size_t i = 0; i < len && str[i] != '\0'; ++i) {
			result = (result ^ static_cast<Uint64>(str[i])) * prime_64_const;
		}

		return result;
	}

	inline constexpr Uint64 StrHash(const char* str) { return detail::fnv1a_64(str); }
}


// Lightweight StringID object
enum class StringID : std::size_t {};


// Run-time function to convert a strings to StringIDs
StringID ToStringID(const char* str) noexcept;

/* 
*  Compile-time operation to convert a string literal to a StringID.
*  Usage: 
*		"hello, world!"_sid ---compile---> StringID{ (some hashed value) }
*/
consteval StringID operator "" _sid(const char* str, std::size_t sz) noexcept {
	return StringID{ detail::fnv1a_n_64(str, sz) };
}



// Default std::hash for StringID is a no-op, since it is already hashed.
template<>
struct std::hash<StringID>
{
	std::size_t operator() (StringID const& sid) const noexcept;
};


/* 
* Default std::hash for C - style strings uses our hash.Debug version automatically
* stores the string in our debug string dictionary.
*/
template<>
struct std::hash<const char*>
{
	std::size_t operator() (const char* const& str) const noexcept;
};


#ifdef STR_DEBUG

/*
* Manually store a string in the debug string dictionary. Useful for when you
* want to use a compile-time StringID in Release build, but still want to have
* the actual string available in Debug build. For example:
*
*		 // this will NOT store anything in the debug dictionary
*		StringID sid = "hello, world"_sid;
*
*		// So, if we're in Debug, we'll add it ourselves:
*		#ifdef _DEBUG
*		DebugInternString("hello, world");
*		#endif
*
* Returns: true if string was added successfully, false otherwise.
*/
bool DebugInternString(const char* str) noexcept;

// Look up a string in our debug string dictionary using a StringID.
[[nodiscard]] const char* DebugStringLookUp(StringID sid) noexcept;

std::pmr::unordered_map<StringID, std::pmr::string> const& DebugStringDictionary() noexcept;

using StringDictionary = std::pmr::unordered_map<StringID, std::pmr::string>;
extern StringDictionary* p_dbg_string_dictionary;

#endif