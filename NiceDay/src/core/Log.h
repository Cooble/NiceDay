#pragma once
#include <ndpch.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/common.h>
#include "spdlog/spdlog.h"

class Log {
public:
	static void init();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
	{
		return s_CoreLogger;
		
	}
private:
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
};
#define ND_TRACE(...)    SPDLOG_LOGGER_TRACE(Log::GetCoreLogger(), __VA_ARGS__)
#define ND_INFO(...)     SPDLOG_LOGGER_INFO(Log::GetCoreLogger(), __VA_ARGS__)
#define ND_WARN(...)     SPDLOG_LOGGER_WARN(Log::GetCoreLogger(), __VA_ARGS__)
#define ND_ERROR(...)    SPDLOG_LOGGER_ERROR(Log::GetCoreLogger(), __VA_ARGS__)
#define ND_WAIT_FOR_INPUT std::cin.get()