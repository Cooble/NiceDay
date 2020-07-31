#include "ndpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;


void Log::init()
{
	static bool noInit = true;
	if (!noInit)
		return;
	noInit = false;
	
	//spdlog::set_pattern("[%@] %^[%T] %n: %v%$");
	//spdlog::set_pattern();

	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log.txt", 1048576*1, 1,true));
	sinks[1]->set_level(spdlog::level::trace);

	sinks[0]->set_pattern("[%=26@]%^  %v%$");
	sinks[1]->set_pattern("[%-7l|%=26@] %v%$");
	
	s_CoreLogger = std::make_shared<spdlog::logger>("ND", begin(sinks), end(sinks));
	s_CoreLogger->set_level(spdlog::level::trace);
	s_CoreLogger->flush_on(spdlog::level::trace);

	
	//register it if you need to access it globally
	spdlog::register_logger(s_CoreLogger);

	ND_INFO("Logger running");
}

void Log::flush()
{
	s_CoreLogger->flush();
}
