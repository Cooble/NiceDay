#include "ndpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"


std::shared_ptr<spdlog::logger> Log::s_CoreLogger;


void Log::init()
{
	static bool noInit = true;
	if (!noInit)
		return;
	noInit = false;
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("[%@] %^[%T] %n: %v%$");
	s_CoreLogger = spdlog::stdout_color_mt("ND");
	s_CoreLogger->set_level(spdlog::level::trace);
	ND_INFO("Logger running");
}
