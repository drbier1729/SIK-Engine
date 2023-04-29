#include "stdafx.h"
#include "LogManager.h"

Vector<spdlog::sink_ptr> LogManager::sinks{};

void LogManager::Init() {
	// A console sink
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_pattern("%^[%Y-%m-%d %a %H:%M:%S.%e] [%l] [%s:%#] %v%$");

	sinks.push_back(console_sink);
	auto logger = std::make_shared<spdlog::logger>(SIK_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
	spdlog::register_logger(logger);
}

void LogManager::Destroy() {
	spdlog::shutdown();
}

void LogManager::ScriptLog(LogLevel log_level, const char* log_msg) {
	switch (log_level) {
	case SIK_TRACE:
		SIK_TRACE(log_msg);
		break;
	case SIK_DEBUG:
		SIK_DEBUG(log_msg);
		break;
	case SIK_INFO:
		SIK_INFO(log_msg);
		break;
	case SIK_WARN:
		SIK_WARN(log_msg);
		break;
	case SIK_ERROR:
		SIK_ERROR(log_msg);
		break;
	case SIK_CRITICAL:
		SIK_CRITICAL(log_msg);
		break;
	default:
		SIK_WARN("Log level not found");
		break;
	}
}