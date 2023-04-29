#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define SIK_DEFAULT_LOGGER_NAME "siklogger"

// Platform specific for windows
#define SIK_BREAK __debugbreak();


/*
* SIK_LOG_LEVEL can be defined to enable logging at various levels:
*	4 == All logging enabled
*   3 == Warn, Error and Critical
*   2 == Error and Critical
*   1 == Critical only
*   0 == All logging disabled
*/
#if SIK_LOG_LEVEL > 0
#define SIK_CRITICAL(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_CRITICAL(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#else
#define SIK_CRITICAL(...) (void)0
#endif

#if SIK_LOG_LEVEL > 1
#define SIK_ERROR(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_ERROR(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#else
#define SIK_ERROR(...) (void)0
#endif

#if SIK_LOG_LEVEL > 2
#define SIK_WARN(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_WARN(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#else
#define SIK_WARN(...) (void)0
#endif

#if SIK_LOG_LEVEL > 3
#define SIK_TRACE(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_TRACE(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#define SIK_DEBUG(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_DEBUG(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#define SIK_INFO(...)	if (spdlog::get(SIK_DEFAULT_LOGGER_NAME) != nullptr) { SPDLOG_LOGGER_INFO(spdlog::get(SIK_DEFAULT_LOGGER_NAME), __VA_ARGS__); }
#else
#define SIK_TRACE(...) (void)0
#define SIK_INFO(...) (void)0
#define SIK_DEBUG(...) (void)0
#endif 

#ifdef _DEBUG	
#define SIK_ASSERT(x, msg)	if ((x)) {} else { SIK_CRITICAL("ASSERT: {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); SIK_BREAK }
#else
#define SIK_ASSERT(x, msg) (void)0
#endif