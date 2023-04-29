#pragma once

/*
* The LogManager class is used to manage sinks for logging.
*/
class LogManager {
private:
	// A logger to log to various sinks like console sink or file sink
	static Vector<spdlog::sink_ptr> sinks;

	// enum for script log levels
	enum LogLevel {
		SIK_TRACE,
		SIK_DEBUG,
		SIK_INFO,
		SIK_WARN,
		SIK_ERROR,
		SIK_CRITICAL
	};

public:
	//Pure static class
	LogManager() = delete;
	LogManager(LogManager const&) = delete;
	LogManager(LogManager&&) = delete;
	LogManager& operator=(LogManager const&) = delete;
	LogManager& operator=(LogManager&&) = delete;
	~LogManager() = delete;

	static void Init();
	static void Destroy();

	static void ScriptLog(LogLevel log_level, const char* log_msg);
};