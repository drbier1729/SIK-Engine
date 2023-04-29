#include "stdafx.h"
#include "StringID.h"



// std::hash<StringID> is now just a cast to size_t, since StringIDs are already hashed
std::size_t std::hash<StringID>::operator() (StringID const& sid) const noexcept {
	return static_cast<std::size_t>(sid);
}


#ifdef STR_DEBUG

std::pmr::unordered_map<StringID, std::pmr::string> const& DebugStringDictionary() noexcept {
	return *p_dbg_string_dictionary;
}

bool DebugInternString(const char* c_string) noexcept {
	// Hash
	std::size_t val = detail::StrHash(c_string);
	StringID sid = StringID{ val };

	// Add to our string dictionary
	auto it = p_dbg_string_dictionary->find(sid);
	bool success = ( it == p_dbg_string_dictionary->end() );
	if (success) {
		success = p_dbg_string_dictionary->emplace(sid, c_string).second;
	}
	
	return success;
}

// std::hash<const char*> now uses XXH3
std::size_t std::hash<const char*>::operator() (const char* const& c_string) const noexcept {
	// Hash
	std::size_t val = detail::StrHash(c_string);
	
	// Add to our static string dictionary
	StringID sid = StringID{ val };
	auto it = p_dbg_string_dictionary->find(sid);
	if (it == p_dbg_string_dictionary->end()) {
		p_dbg_string_dictionary->emplace(sid, c_string).second;
	}

	return val;
}


[[nodiscard]] const char* DebugStringLookUp(StringID sid) noexcept {
	auto it = p_dbg_string_dictionary->find(sid);
	if (it != p_dbg_string_dictionary->end()) {
		return it->second.c_str();
	}

	return nullptr;
}

#else

// std::hash<const char*> now actually does a hash based on string contents, not just
// the pointer
std::size_t std::hash<const char*>::operator() (const char* const& c_string) const noexcept {
	std::size_t len = (c_string == nullptr) ? 0ull : std::strlen(c_string);
	return static_cast<std::size_t>( detail::StrHash(c_string) );
}

#endif

// Run-time conversion
StringID ToStringID(const char* c_string) noexcept {
	return StringID{ std::hash<const char*>{}(c_string) };
}
