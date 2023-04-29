#pragma once

class XORShift64 {
private:
	// If state == 0, this will only ever generate a result of 0.
	Uint64 state = 0;

public:
	explicit XORShift64(Uint64 initial_state) : state{ initial_state } {}

	inline void SetState(Uint64 new_state) { state = new_state; }
	inline Uint64 GetState() const { return state; }

	// Returns a 64-bit unsigned integer from a uniform distribution
	inline Uint64 operator()() {
		state ^= state >> 12;
		state ^= state << 25;
		state ^= state >> 27;
		return state * 2685821657736338717ull;
	}

	// Returns a double between 0 and 1 from a uniform distribution
	inline Float64 Double() {
		return XORShift64::operator()() / 18446744073709551616.0;
	}
};

// Templated Base Class
template<class T>
class RandomGenerator {
public:
	using ArgT = std::conditional_t<(sizeof(T) > sizeof(T*)), T const&, T>;

	// Generates a random value of type T
	virtual T Gen() { return T{}; }
	virtual T GenInRange(ArgT lo, ArgT hi) { return T{lo}; }
	
	// Sets the seed for the random number generator. These may be overrided 
	// depending on the internals of subclasses.
	virtual void SetSeed(Uint64 sd) { }
	virtual Uint64 GetSeed() const { return 0; }
};


// Concrete implementations
class UniformRandFloat32 : public RandomGenerator<Float32> {
private:
	XORShift64 engine;

public:
	explicit UniformRandFloat32(Uint64 seed = 1ull) : engine{ seed } {}

	inline Float32 Gen() override {
		return static_cast<Float32>(engine.Double());
	}
	inline Float32 GenInRange(Float32 lo, Float32 hi) override {
		return static_cast<Float32>(engine.Double()) * (hi - lo) + lo;
	}
	inline void    SetSeed(Uint64 sd) override {
		engine.SetState(sd);
	}
	inline Uint64  GetSeed() const override {
		return engine.GetState();
	}
};


// GLM Helpers
inline Vec2 RandVec2(Vec2 const& lo, Vec2 const& hi, RandomGenerator<Float32>& rand_float) {
	return Vec2(rand_float.GenInRange(lo.x, hi.x),
		rand_float.GenInRange(lo.y, hi.y));
}

inline Vec3 RandVec3(Vec3 const& lo, Vec3 const& hi, RandomGenerator<Float32>& rand_float) {
	return Vec3(rand_float.GenInRange(lo.x, hi.x),
		rand_float.GenInRange(lo.y, hi.y),
		rand_float.GenInRange(lo.z, hi.z));
}

inline Vec4 RandVec4(Vec4 const& lo, Vec4 const& hi, RandomGenerator<Float32>& rand_float) {
	return Vec4(rand_float.GenInRange(lo.x, hi.x),
		rand_float.GenInRange(lo.y, hi.y),
		rand_float.GenInRange(lo.z, hi.z),
		rand_float.GenInRange(lo.w, hi.w));
}