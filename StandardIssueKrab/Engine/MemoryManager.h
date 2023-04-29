#pragma once

/*
* This class manages all allocations throughout the program by holding
* handles to one or more polymorphic memory resources.
*/
class MemoryManager
{
private:
	// TODO : create interface for an "allocator stack"
	MemoryResource* current_resource = std::pmr::get_default_resource();

public:
	// Ctors and Dtor defaulted

	// Returns a polymorphic allocator that allocates from the current_resource
	PolymorphicAllocator GetCurrentAllocator() const noexcept {
		return std::pmr::polymorphic_allocator<>(current_resource);
	}

	// Helper to convert default resource into a pmr allocator that uses it
	static PolymorphicAllocator GetDefaultAllocator() noexcept {
		return std::pmr::polymorphic_allocator<>(std::pmr::get_default_resource());
	}
};

//Declared as an extern variable so it can be accessed throughout the project
extern MemoryManager* p_memory_manager;