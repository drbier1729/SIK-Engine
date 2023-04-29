#include "stdafx.h"
#include "GOandCompTest.h"

#include "Engine/MemoryResources.h"
#include "Engine/GameObject.h"
#include "Engine/Component.h"
#include "Engine/Serializer.h"
#include "Engine/TestComp.h"


// TODO (bug) : this test leaks 200 bytes. So does SerializeTest (264 bytes) and
// ScriptTest (120 bytes. Maybe they're related...

void GOandCompTest::Setup(EngineExport* p_engine_export_struct) {
#ifdef STR_DEBUG
	p_dbg_string_dictionary = p_engine_export_struct->p_dbg_string_dictionary;
#endif
	SetRunning();
}

void GOandCompTest::Run() {

	PolymorphicAllocator alloc{ std::pmr::get_default_resource() }; // noisy allocator

	{
		GameObject go{ "Test Object", alloc };
		
		{
			TestComp2* comp = alloc.new_object<TestComp2>();
			go.AddComponent(comp);
			{
				auto* p = go.HasComponent<TestComp2>();
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent("TestComp2"_sid);
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent(TestComp2::type);
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent("TestComp2");
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
		}

		{
			TestComp* comp = alloc.new_object<TestComp>();
			go.AddComponent(comp); // go takes ownership of comp

			{
				auto* p = go.HasComponent<TestComp>();
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent("TestComp"_sid);
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent(TestComp::type);
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
			{
				auto* p = go.HasComponent("TestComp");
				SIK_ASSERT(p == comp, "Ptrs should be the same.");
			}
		}
		
		auto const& components = go.GetComponentArray();
		SIK_INFO("Components for object {}", go.GetName());
		for (auto const & c : components) { 
			if (c != nullptr) {
				SIK_INFO("{}: SID = {}, TypeID = {}",
					c->GetName(),
					static_cast<Uint64>(c->GetNameSID()),
					static_cast<Uint64>(c->GetType()));
			}
		}
	}

	SetPassed();
}

void GOandCompTest::Teardown() {
#ifdef STR_DEBUG
	p_dbg_string_dictionary = nullptr;
#endif
	SetPassed();
}