#include "stdafx.h"
#include "MemoryResources.h"

////////////////////////////////////////////////////////////////////////////////
// Debug Memory Resource
////////////////////////////////////////////////////////////////////////////////
DebugMemoryResource::DebugMemoryResource(const char* name, Flag flag, std::pmr::memory_resource* upstream)
	: upstream_{ upstream }, name_{ name }, 
	currently_allocated_bytes_{ 0 }, flag_ { flag }	
{ }

DebugMemoryResource::~DebugMemoryResource() noexcept {
	if (currently_allocated_bytes_ > 0) {
		SIK_ERROR("Not all bytes were freed from memory resource \"{}\"! Lost {} bytes.", name_, currently_allocated_bytes_);
	}
}


void* DebugMemoryResource::do_allocate(size_t bytes, size_t alignment) {
	void* ptr = upstream_->allocate(bytes, alignment);
	if (flag_ == Flag::LogAndTrack) {
		SIK_INFO("{}: + Allocating {} bytes @ {}", name_, bytes, ptr);
	}
	currently_allocated_bytes_ += bytes;
	return ptr;
}

void DebugMemoryResource::do_deallocate(void* ptr, size_t bytes, size_t alignment) {
	if (flag_ == Flag::LogAndTrack) {
		SIK_INFO("{}: - Deallocating {} bytes @ {}", name_, bytes, ptr);
	}
	currently_allocated_bytes_ -= bytes;
	upstream_->deallocate(ptr, bytes, alignment);
}

bool DebugMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
	return upstream_->is_equal(other);
}

////////////////////////////////////////////////////////////////////////////////
// Linear Memory Resource
////////////////////////////////////////////////////////////////////////////////
LinearMemoryResource::LinearMemoryResource(void* data, std::size_t capacity) 
	: data_ { static_cast<std::byte*>(data) },
	top_{data_},
	capacity_{capacity}
{ }

void* LinearMemoryResource::do_allocate(std::size_t bytes, std::size_t alignment) {

	void* marker{ reinterpret_cast<void*>(top_) };
	std::size_t remaining = capacity_ - Size();

	if ( std::align(alignment, bytes, marker, remaining) != nullptr ) [[likely]] {
		top_ = static_cast<std::byte*>(marker) + bytes;
		return marker;
	}
	else {
		throw std::bad_alloc{};
	}
}
