#include "stdafx.h"

#include "MemoryResourcesTest.h"
#include "Engine/MemoryResources.h"
#include "Engine/FrameTimer.h"

void MemoryResourcesTest::Setup(EngineExport*) {
	SIK_INFO("Setup Passed");
	SetRunning();
	return;
}

void MemoryResourcesTest::Run() {

	// Linear Memory Resource
	{
		LinearMemoryResource linear{ buffer.data(), buffer.size() };
		PolymorphicAllocator alloc{ &linear };

		// Uncomment these if you want allocation and deallocation calls to be logged
		//DebugMemoryResource dbg_linear{ "Linear", &linear };
		//std::pmr::polymorphic_allocator<> alloc{ &dbg_linear };

		Int32* a = alloc.new_object<Int32>(0);
		Float32* b = alloc.new_object<Float32>(10.0f);
		SizeT* c = alloc.new_object<SizeT>(20);
		Float64* d = alloc.new_object<Float64>(30.0);
		Array<char, 128>* e = alloc.new_object<Array<char, 128>>();

		for (auto i = 0; i < 256; ++i) {
			auto p = alloc.new_object<Int32>(1);
		}

		for (auto i = 0; i < e->size(); ++i) {
			(*e)[i] = 'a' + i;
		}

		linear.Clear();

		if (*a != 0 ||
			*b != 0.0f ||
			*c != 0ull ||
			*d != 0.0) {

			SIK_INFO("a = {}, b = {}, c = {}, d = {}", *a, *b, *c, *d);
			SIK_ERROR("Expected the buffer to be zero'd but it was not.");
			SetFailed();
			return;
		}

		for (auto&& ch : *e) {
			if (ch != 0) {
				SIK_ERROR("Expected the buffer to be zero'd but it was not.");
				SetFailed();
				return;
			}
		}
	} // Buffer zero'd by destructor for LinearMemoryResource

	// MultiBuffer Memory Resource 1
	{
		MultiBufferMemoryResource<2> multi_buf{ buffer.data(), buf_size };
		PolymorphicAllocator alloc{ &multi_buf };

		// Uncomment these if you want allocation and deallocation calls to be logged
		//DebugMemoryResource dbg_multi{ "MultiBuf", &multi_buf };
		//std::pmr::polymorphic_allocator<> alloc{ &dbg_multi };

		Int32* a = alloc.new_object<Int32>(0);
		Float32* b = alloc.new_object<Float32>(10.0f);
		SizeT* c = alloc.new_object<SizeT>(20);
		Float64* d = alloc.new_object<Float64>(30.0);
		Array<char, 128>* e = alloc.new_object<Array<char, 128>>();

		for (auto i = 0; i < 256; ++i) {
			auto p = alloc.new_object<Int32>(1);
		}

		for (int i = 0; i < 26; ++i) {
			(*e)[i] = 'a' + i;
		}

		multi_buf.SwapBuffers();
		if (*a != 0 ||
			*b != 10.0f ||
			*c != 20ull ||
			*d != 30.0) {

			SIK_INFO("a = {}, b = {}, c = {}, d = {}", *a, *b, *c, *d);
			SIK_ERROR("Expected the buffer to still exist but it was corrupted.");
			SetFailed();
			return;
		}


		for (auto i = 0; i < 26; ++i) {
			if ((*e)[i] != ('a' + i)) {
				SIK_ERROR("Expected the buffer to still exist but it was corrupted.");
				SetFailed();
				return;
			}
		}

		multi_buf.SwapBuffers();

		if (*a != 0 ||
			*b != 0.0f ||
			*c != 0ull ||
			*d != 0.0) {

			SIK_INFO("a = {}, b = {}, c = {}, d = {}", *a, *b, *c, *d);
			SIK_ERROR("Expected the buffer to be zero'd but it was not.");
			SetFailed();
			return;
		}

		for (auto&& ch : *e) {
			if (ch != 0) {
				SIK_ERROR("Expected the buffer managed by MultiBuffer to be zero'd but it was not.");
				SetFailed();
				return;
			}
		}
	} // Buffer zero'd by destructor for LinearMemoryResource
		

	// MultiBuffer Memory Resource 2
	{
		MultiBufferMemoryResource<16> multi_buf{ buffer.data(), buf_size };
		PolymorphicAllocator alloc{ &multi_buf };

		// Uncomment these if you want allocation and deallocation calls to be logged
		//DebugMemoryResource dbg_multi{ "MultiBuf", &multi_buf };
		//std::pmr::polymorphic_allocator<> alloc{ &dbg_multi };

		for (char i = 0; i < multi_buf.BufferCount() - 1; ++i) {
			for (SizeT j = 0ull; j < multi_buf.BufferCapacity(); ++j) {
				auto p = alloc.new_object<char>('a' + i);
			}
			multi_buf.SwapBuffers();
		}

		for (SizeT j = 0ull; j < multi_buf.BufferCapacity()/sizeof(Float64); ++j) {
			auto p = alloc.new_object<Float64>(3.14);
		}
	} // Buffer zero'd by destructor for LinearMemoryResource

	// Chunk Memory Resource
	{
		LinearMemoryResource upstream{buffer.data(), buffer.size()};
		//DebugMemoryResource dbg_upstream{"Upstream Linear", &upstream};
		ChunkMemoryResource chunk{ 128ull, &upstream };
		//DebugMemoryResource dbg_chunk{ "Chunk", &chunk };

		PolymorphicAllocator alloc{ &chunk };

		// Fill up first chunk
		for (auto i = 0; i < 128; ++i) {
			auto p = alloc.new_object<char>('a');
		}

		// Allocate new chunks and fill them
		for (auto i = 0; i < 100; ++i) {
			auto p = alloc.new_object<double>(3.14);
		}
	} // Buffer zero'd

	// Pool Memory Resource 1
	{
		//DebugMemoryResource dbg_new_delete{ "New-Delete", std::pmr::new_delete_resource() };
		std::pmr::memory_resource* new_delete{ std::pmr::new_delete_resource() };
		
		PoolMemoryResource pool{ new_delete };
		
		PolymorphicAllocator alloc{ &pool };

		List<std::array<char, 64>> lst{ alloc };

		// Normally this would allocate from new_delete 5000 times
		for (auto i = 0; i < 5000; ++i) {
			lst.emplace_back();
		}

		for (auto i = 0; i < 3000; ++i) {
			lst.pop_front();
		}

		// Notice that no actual allocations happen here from new_delete!
		for (auto i = 0; i < 3000; ++i) {
			lst.emplace_front();
		}
	}

	// Pool Memory Resource 2
	{
		//DebugMemoryResource dbg_new_delete{ "New-Delete", std::pmr::new_delete_resource() };
		std::pmr::memory_resource* new_delete{ std::pmr::new_delete_resource() };

		std::pmr::monotonic_buffer_resource chunk{ 64 * 1024, new_delete };
		std::pmr::unsynchronized_pool_resource pool{ &chunk };


		List<Vector<String>> lst{ &pool };
		std::list<std::vector<std::string>> lst_std;

		// std::pmr::list
		{
			SIK_TIMER("Adding 100000 elements to pmr list...");
			for (auto i = 0; i < 100000; ++i) {
				lst.emplace_back(Vector<String>{ "hello world, it's me", "jiminy cricket" });
			}
		}

		{
			SIK_TIMER("Popping 60000 elements to pmr list");
			for (auto i = 0; i < 60000; ++i) {
				lst.pop_front();
			}
		}

		{
			SIK_TIMER("Adding 60000 elements back pmr list");
			for (auto i = 0; i < 60000; ++i) {
				lst.emplace_front(Vector<String>{ "hi, everyone! it's me,", "catherine zeta jones" });
			}
		}

		{
			SIK_TIMER("Iterating and modifying all elements of pmr list");
			for (auto&& vec : lst) {
				vec[0] = "goodbye, y'all! it's been lovely.";
			}
		}

		// std::list
		{
			SIK_TIMER("Adding 100000 elements to std list");
			for (auto i = 0; i < 100000; ++i) {
				lst_std.emplace_back(std::vector<std::string>{ "hello world, it's me", "jiminy cricket" });
			}
		}
		
		{
			SIK_TIMER("Popping 60000 elements to std list");
			for (auto i = 0; i < 60000; ++i) {
				lst_std.pop_front();
			}
		}

		{
			SIK_TIMER("Adding 60000 elements back std list");
			for (auto i = 0; i < 60000; ++i) {
				lst_std.emplace_front(std::vector<std::string>{ "hi, everyone! it's me,", "catherine zeta jones" });
			}
		}
		
		{
			SIK_TIMER("Iterating and modifying all elements of std list");
			for (auto&& vec : lst_std) {
				vec[0] = "goodbye, y'all! it's been lovely.";
			}
		}
	}

	SIK_INFO("Test Passed");
	SetPassed();
	return;
}

void MemoryResourcesTest::Teardown() {
	SIK_INFO("Teardown Passed");
	SetPassed();
	return;
}