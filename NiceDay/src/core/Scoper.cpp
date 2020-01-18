#include "ndpch.h"
#include "Scoper.h"

void Scoper::beginSession(const std::string& name, const std::string& filepath)
{
	if (m_outputStream.is_open())
	{
		ND_WARN("Replacing current profiling session: {} with: {}", m_currentSession->name, name);
		endSession();
	}
	auto filep = "profiles/" + filepath;
	if (filep.find_last_of('/') != std::string::npos)
	{
		auto s = filep.substr(0, filep.find_last_of('/'));
		if (!std::filesystem::exists(s))
			std::filesystem::create_directories(s);
	}
	m_outputStream.open(filep);
	writeHeader();
	m_currentSession = new ScopeSession{name};
}
