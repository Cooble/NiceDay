#pragma once
#include <ndpch.h>
#include "spdlog/spdlog.h"

class Log {
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
private:
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
};
#define ND_TRACE(...)    Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ND_INFO(...)     Log::GetCoreLogger()->info(__VA_ARGS__)
#define ND_WARN(...)     Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ND_ERROR(...)    Log::GetCoreLogger()->error(__VA_ARGS__)
#define ND_FATAL(...)    Log::GetCoreLogger()->fatal(__VA_ARGS__)
#define ND_WAIT_FOR_INPUT std::cin.get()