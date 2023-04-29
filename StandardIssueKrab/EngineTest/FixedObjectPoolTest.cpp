#include "stdafx.h"
#include "FixedObjectPoolTest.h"

#include "Engine/FixedObjectPool.h"

struct ThirtyTwoBytes
{
	double a = 0.0;
	double b = 0.0;
	std::size_t c = 0;
	std::size_t d = 0;

	void print() const noexcept {
		SIK_WARN("Elem: a = {}, b = {}, c = {}, d = {}",a,b,c,d);
	}
};

template<class T, std::size_t N>
using FOP = FixedObjectPool<T, N>;

static void test0() {
	FixedObjectPool<ThirtyTwoBytes, 32> fop{};

	ThirtyTwoBytes y{ .a = 1.41, .d = 20 };
	auto p1 = fop.insert(y);

	y.a = 3.14;
	y.d = 40;
	auto p2 = fop.insert(y);

	y.b = 3.14;
	y.c = 40;
	auto p3 = fop.insert(y);

	auto p4 = fop.erase(p2);

	y.a = 1.1111;
	y.b = 2.2222;
	y.c = 333333;
	y.d = 444444;
	auto p5 = fop.insert(y);
	auto p6 = fop.insert(y);

	auto range = fop.all();
	while (!range.is_empty()) {
		auto const& elem = range.front();
		elem.print();
		range.pop_front();
	}
}

static void test1() {
	auto p_fop = std::make_unique<FixedObjectPool<int, 100>>();

	std::vector<int*> vec{};
	vec.reserve(100);

	for (int i = 0; i < 50; ++i) {
		vec.push_back(p_fop->insert(i));
	}

	SIK_WARN("-------------Added 50 ints-------------");
	int count = 0;
	auto range = p_fop->all();
	while (!range.is_empty()) {
		SIK_WARN("Elem {} = {}", count, range.front());
		count++;
		range.pop_front();
	}

	for (int i = 0; i < 50; i += 4) {

		auto& p = vec[std::rand() % 50];
		if (p != nullptr) {
			p_fop->erase(p);
			p = nullptr;
		}
	}

	vec.erase(std::remove_if(vec.begin(),
		vec.end(),
		[](int* p) { return (p == nullptr); }),
		vec.end());

	SIK_WARN("-------------Erased some-------------");
	count = 0;
	range = p_fop->all();
	while (!range.is_empty()) {
		SIK_WARN("Elem {} = {}", count, range.front());
		count++;
		range.pop_front();
	}

	for (int i = 0; i < 50; ++i) {
		vec.push_back(p_fop->insert(i * 10000));
	}


	SIK_WARN("-------------Added 50 more ints-------------");
	count = 0;
	range = p_fop->all();
	while (!range.is_empty()) {
		SIK_WARN("Elem {} = {}", count, range.front());
		count++;
		range.pop_front();
	}
}

static void test2() {

	auto p_fop = std::make_unique<FOP<std::vector<ThirtyTwoBytes>, 100ull>>();

	std::vector<std::vector<ThirtyTwoBytes>*> vec;

	for (std::size_t i = 0ull; i < p_fop->capacity(); ++i) {
		std::vector<ThirtyTwoBytes> x(10, ThirtyTwoBytes{
			.a = i * 3.14,
			.b = i + 3.14,
			.c = i, .
			d = 100 * i });

		// Copy the vector, x
		vec.push_back(p_fop->insert(x));
	}

	SIK_WARN("-------------Added 100 vectors of ThirtyTwoBytes-------------");
	std::size_t count = 0;
	auto range = p_fop->all();
	while (!range.is_empty()) {

		SIK_WARN("//////// Vector #{} /////////", count);
		count++;
		for (auto&& val : range.front()) {
			val.print();
		}

		range.pop_front();
	}

	// Erasing some elements
	for (std::size_t i = 0ull; i < p_fop->capacity() / 2; i += 4ull) {

		auto& p = vec[std::rand() % p_fop->capacity() / 2];
		if (p != nullptr) {
			p_fop->erase(p);
			p = nullptr;
		}
	}

	vec.erase(std::remove_if(vec.begin(),
		vec.end(),
		[](auto* p) { return (p == nullptr); }),
		vec.end());

	SIK_WARN("-------------Erased some vectors-------------");
	count = 0;
	range = p_fop->all();
	while (!range.is_empty()) {

		SIK_WARN("//////// Vector #{} /////////", count);
		count++;
		for (auto&& val : range.front()) {
			val.print();
		}

		range.pop_front();
	}

	std::size_t remaining_capacity = p_fop->capacity() - p_fop->size();
	for (std::size_t i = 0ull; i < remaining_capacity; ++i) {
		std::vector<ThirtyTwoBytes> x(10, ThirtyTwoBytes{
			.a = i * 10000000.0,
			.b = i + 10000000.0,
			.c = i, .
			d = 100000000000000 * i });

		// Copy the vector, x
		vec.push_back(p_fop->insert(x));
	}

	SIK_WARN("-------------Inserted more vectors-------------");

	count = 0;
	range = p_fop->all();
	while (!range.is_empty()) {

		SIK_WARN("//////// Vector #{} /////////", count);
		count++;
		for (auto&& val : range.front()) {
			val.print();
		}

		range.pop_front();
	}
}

static void test3() {

	FOP<float, 1000> fop{};

	std::vector<float*> ptrs{};
	ptrs.reserve(1000);

	for (int i = 0; i < 50; ++i) {
		float* p = fop.insert(i * 3.14f);
		ptrs.push_back(p);
	}

	int counter = 0;
	for (auto r = fop.all(); !r.is_empty(); r.pop_front()) {
		SIK_WARN("Elem {} = {}", counter, r.front()); 
		counter++;
	}

	fop.erase(ptrs[10]);
	ptrs[10] = nullptr;

	counter = 0;

	for (auto r = fop.all(); !r.is_empty(); r.pop_front()) {
		SIK_WARN("Elem {} = {}", counter, r.front());
		counter++;
	}

	float* p = fop.insert(10000000 * 3.14f);

	counter = 0;
	for (auto r = fop.all(); !r.is_empty(); r.pop_front()) {
		SIK_WARN("Elem {} = {}", counter, r.front());
		counter++;
	}

	counter = 0;
	for (auto r_i = fop.all(); !r_i.is_empty(); r_i.pop_front()) {

		auto r_j = r_i;
		r_j.pop_front();

		for (; !r_j.is_empty(); r_j.pop_front()) {
			SIK_WARN("Diff {} = {}", counter, r_j.front() - r_i.front());
			counter++;
		}
	}
}

static void test4() {

	// Copy constructor not implemented

	//FOP<float, 1000> fop{};

	//std::vector<float*> ptrs{};
	//ptrs.reserve(1000);

	//for (int i = 0; i < 50; ++i) {
	//	float* p = fop.insert(i * 3.14f);
	//	ptrs.push_back(p);
	//}

	//// Copy ctor
	//FOP<float, 1000> fop2{ fop };

	//int counter = 0;

	//SIK_WARN("/////////////// FOP #1 ////////////");
	//counter = 0;
	//for (auto r = fop.all(); !r.is_empty(); r.pop_front()) {
	//	SIK_WARN("Elem {} = {}", counter, r.front());
	//	counter++;
	//}

	//SIK_WARN("/////////////// FOP #2 ////////////");
	//counter = 0;
	//for (auto r = fop2.all(); !r.is_empty(); r.pop_front()) {
	//	SIK_WARN("Elem {} = {}", counter, r.front());
	//	counter++;
	//}

}

static void test5() {

	// Move constructor not implemented

	/*
	FOP<std::vector<ThirtyTwoBytes>, 20ull> fop{};

	std::vector<std::vector<ThirtyTwoBytes>*> vec;

	for (std::size_t i = 0ull; i < fop.capacity(); ++i) {
		vec.push_back(fop.emplace(
			10,
			ThirtyTwoBytes{ .a = i * 3.14, .b = i + 3.14, .c = i, .d = 100 * i }
		));
	}

	decltype(fop) fop2{ std::move(fop) };

	SIK_WARN("-------------Moved from fop with 100 vectors of ThirtyTwoBytes-------------");
	std::size_t count = 0;
	auto range = fop2.all();
	while (!range.is_empty()) {

		SIK_WARN("//////// Vector #{} //////////", count);
		count++;
		for (auto&& val : range.front()) {
			val.print();
		}

		range.pop_front();
	}*/
}

static void test6() {

	FOP<double, 1024> fop{};

	Vector<double*> ptrs(1024, nullptr);

	for (auto i = 0; i < 512; ++i) {
		ptrs[i] = fop.emplace(i * 10.0);
	}

	auto const& c_fop = fop;

	for (auto r = c_fop.all(); !r.is_empty(); r.pop_front()) {
		auto& d = r.front();
		SIK_INFO("d * 3.14 = {}", d * 3.14);
	}
}

struct FOPTest {
	int num;
	decltype(&test0) test;
};

static FOPTest pTests[] = {
	{ 0, test0 },
	{ 1, test1 },
	{ 2, test2 },
	{ 3, test3 },
	{ 4, test4 },
	{ 5, test5 },
	{ 6, test6 }
};


void FixedObjectPoolTest::Setup(EngineExport*) {
	SetRunning();
	return;
}

void FixedObjectPoolTest::Run() {
	static constexpr int num_tests = sizeof(pTests) / sizeof(FOPTest);

	for (int i = 0; i < num_tests; ++i) {
		SIK_INFO("////////////////Running Test #{}////////////////", pTests[i].num);
		pTests[i].test();
	}
	SetPassed();
	return;
}

void FixedObjectPoolTest::Teardown() {
	SetPassed();
	return;
}
