#pragma once

/*
* The FrameTimer class is a thin wrapper around std::chrono timing capabilities
* using steady_clock, and provides methods to access and adjust the fixed delta time
* of the game update loop.
*/
class FrameTimer
{
public:
	using duration_type = std::chrono::nanoseconds;

private:
	using clock = std::chrono::steady_clock;

private:
	/*
	* Dictates simulation time of one step the game's update and physics loops.
	* This must be tuned so that it is as small as possible, but still longer
	* than the real-world time it takes to update the game logic and physics.
	*/
	duration_type fixed_delta_time;

public:
	// Ctors and Dtor defaulted
	
	// Returns current steady_clock time as a time_point
	static auto Now() {
		return clock::now();
	}


	// Calculates the ratio needed to multiply duration.count() by in order to such that
	// the result will have the units of seconds.
	// Template function : Can pass any std::chrono::duration as argument.
	template<std::floating_point RetType>
	static RetType GetToSecondsRatio(std::convertible_to<duration_type> auto duration) {
		return (static_cast<RetType>(decltype(duration)::period::num) / decltype(duration)::period::den);
	}

	// Converts a duration with precision <= duration_type to seconds measured as RetType.
	// Template function : Can pass any std::chrono::duration as argument.
	template<std::floating_point RetType>
	static RetType ToSeconds(std::convertible_to<duration_type> auto duration) {
		return  duration.count() * GetToSecondsRatio<RetType>(duration);
	}

	// Returns the duration of fixed_delta_time
	duration_type GetFixedDeltaTime() {
		return fixed_delta_time;
	}

	// Sets the duration of fixed_delta_time. Template function: Can pass any std::chrono::duration as argument.
	void SetFixedDeltaTime(std::convertible_to<duration_type> auto dt) {
		fixed_delta_time = dt;
	}
};

struct ScopeTimer {
	ScopeTimer(String message, const char* current_file = "", Int64 current_line = 0)
		: msg{ std::move(message) }, file{ current_file }, line{ current_line },
		start{ FrameTimer::Now() } 
	{}
	
	~ScopeTimer() noexcept { 
		auto elapsed = FrameTimer::Now() - start;
		if (line != 0) {
			SIK_INFO("TIMED SCOPE @ {}:{} [ {}ms ] -- message: \"{}\"",
				file.filename().string().c_str(),
				line,
				std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
				msg);
		}
		else {
			SIK_INFO("TIMED SCOPE [ {}ms ] -- message: \"{}\"",
				std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
				msg);
		}
	}
	String msg;
	std::filesystem::path file;
	Int64 line;
	decltype(FrameTimer::Now()) start;
};

#ifdef _DEBUG
#define SIK_TIMER(msg) ScopeTimer annoyingly_bad_timer_name0123456{ msg, __FILE__, __LINE__ }
#else
#define SIK_TIMER(msg) void(0)
#endif

extern FrameTimer* p_frame_timer;
