#pragma once
#include <ndpch.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/common.h>
#include "spdlog/spdlog.h"
#include <spdlog/fmt/bundled/ostream.h>

namespace nd {
class Log
{
public:
	static void init();
	static void flush();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
	{
		return s_CoreLogger;
	}

private:
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
};

#define ND_BUG(...)    SPDLOG_LOGGER_DEBUG(::nd::Log::GetCoreLogger(), __VA_ARGS__)
#define ND_TRACE(...)    SPDLOG_LOGGER_TRACE(::nd::Log::GetCoreLogger(), __VA_ARGS__)
#define ND_INFO(...)     SPDLOG_LOGGER_INFO(::nd::Log::GetCoreLogger(), __VA_ARGS__)
#define ND_WARN(...)     SPDLOG_LOGGER_WARN(::nd::Log::GetCoreLogger(), __VA_ARGS__)
#define ND_ERROR(...)    SPDLOG_LOGGER_ERROR(::nd::Log::GetCoreLogger(), __VA_ARGS__)
#define ND_WAIT_FOR_INPUT std::cin.get()


template <typename T>
std::ostream& operator<<(std::ostream& ost, const glm::vec<2, T>& d)
{
	ost << fmt::format("Vec2=({}, {})", d.x, d.y);
	return ost;
}

template <typename T>
std::ostream& operator<<(std::ostream& ost, const glm::vec<3, T>& d)
{
	ost << fmt::format("Vec3=({}, {}, {})", d.x, d.y, d.z);
	return ost;
}

template <typename T>
std::ostream& operator<<(std::ostream& ost, const glm::vec<4, T>& d)
{
	ost << fmt::format("Vec4=({}, {}, {}, {})", d.x, d.y, d.z, d.w);
	return ost;
}
}
