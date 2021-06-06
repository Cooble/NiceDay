#include "ndpch.h"
#include "Scoper.h"
#include "files/FUtil.h"

namespace nd {
void Scoper::beginSession(const std::string& name, const std::string& filepath)
{
	if (m_outputStream.is_open())
	{
		ND_WARN("Replacing current profiling session: {} with: {}", m_currentSession->name, name);
		endSession();
	}

	auto filep = FUtil::getExecutableFolderPath() + "/profiles/" + filepath;
	if (filep.find_last_of('/') != std::string::npos)
	{
		auto s = filep.substr(0, filep.find_last_of('/'));
		if (!std::filesystem::exists(s))
			std::filesystem::create_directories(s);
	}
	m_outputStream.open(filep);
	writeHeader();

	m_currentSession = new ScopeSession{name};
	ND_TRACE("Begging profiling session: {}", m_currentSession->name);
}

void Scoper::endSession()
{
	if (m_currentSession)
	{
		writeFooter();
		m_outputStream.close();
		ND_TRACE("Ending profiling session: {}", m_currentSession->name);
		delete m_currentSession;
		m_currentSession = nullptr;
		m_profileCount = 0;
	}
	else { ND_WARN("Calling end session, but none is opened"); }
}

/*static void removeInvalidChars(std::string& s)
{
for (int i = 0; i < s.size(); ++i)
{
auto& ss = s[i];
if (ss < 32 || ss>126 || s == '\\') {
ss = ' ';
ASSERT(false, "Gotcha");
}
}
}*/
// every n writes flush everything
constexpr int FLUSH_INTERVAL = 30;

void Scoper::writeProfile(const ProfileResult& result)
{
	if (!m_currentSession)
		return; //no session
	if (m_profileCount++ > 0)
		m_outputStream << ",";

	std::string name = result.name;
	std::replace(name.begin(), name.end(), '"', '\'');
	//removeInvalidChars(name);

	m_outputStream << "{";
	m_outputStream << "\"cat\":\"function\",";
	m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
	m_outputStream << "\"name\":\"" << name << "\",";
	m_outputStream << "\"ph\":\"X\",";
	m_outputStream << "\"pid\":0,";
	m_outputStream << "\"tid\":" << result.threadID << ",";
	m_outputStream << "\"ts\":" << result.start;
	m_outputStream << "}";

	static int flushInterval = FLUSH_INTERVAL;
	if (--flushInterval == 0)
	{
		flushInterval = FLUSH_INTERVAL;
		m_outputStream.flush();
	}
}

void Scoper::writeHeader()
{
	m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
	m_outputStream.flush();
}

void Scoper::writeFooter()
{
	m_outputStream << "]}";
	m_outputStream.flush();
}

void ScoperTimer::stop()
{
	auto endTimepoint = std::chrono::high_resolution_clock::now();


	long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint)
	                .time_since_epoch().count();

	uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
	Scoper::get().writeProfile({m_name, m_startTimepoint, end, threadID});

	m_stopped = true;
}
}
