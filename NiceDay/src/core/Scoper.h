#pragma once
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>


struct ProfileResult
{
	std::string name;
	long long start, end;
	uint32_t threadID;
};

struct ScopeSession
{
	std::string name;
};

class Scoper
{
private:
	ScopeSession* m_currentSession;
	std::ofstream m_outputStream;
	int m_profileCount;
public:
	Scoper()
		: m_currentSession(nullptr), m_profileCount(0)
	{
	}

	void beginSession(const std::string& name, const std::string& filepath = "results.json");

	void endSession()
	{
		writeFooter();
		m_outputStream.close();
		delete m_currentSession;
		m_currentSession = nullptr;
		m_profileCount = 0;
	}

	void writeProfile(const ProfileResult& result)
	{
		if (!m_currentSession)
			return;//no session
		if (m_profileCount++ > 0)
			m_outputStream << ",";

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_outputStream << "{";
		m_outputStream << "\"cat\":\"function\",";
		m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_outputStream << "\"name\":\"" << name << "\",";
		m_outputStream << "\"ph\":\"X\",";
		m_outputStream << "\"pid\":0,";
		m_outputStream << "\"tid\":" << result.threadID << ",";
		m_outputStream << "\"ts\":" << result.start;
		m_outputStream << "}";

		m_outputStream.flush();
	}

	void writeHeader()
	{
		m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
		m_outputStream.flush();
	}

	void writeFooter()
	{
		m_outputStream << "]}";
		m_outputStream.flush();
	}

	static Scoper& get()
	{
		static Scoper instance;
		return instance;
	}
};

class ScoperTimer
{
private:
	const char* m_name;
	long long m_startTimepoint;
	bool m_stopped;
	
public:
	ScoperTimer(const char* name)
		: m_name(name), m_stopped(false)
	{
		auto now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now())
			.time_since_epoch().count();
		
		thread_local long long lastStart = 0;
		
		m_startTimepoint = now;
		
		if(m_startTimepoint==lastStart)//we must not allow two timers to start at once (due to high granularity of high_res_clock)
		{
			++m_startTimepoint;
		}
		lastStart = m_startTimepoint;
		
	}

	~ScoperTimer()
	{
		if (!m_stopped)
			stop();
	}

	void stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

	
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint)
		                .time_since_epoch().count();

		uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Scoper::get().writeProfile({m_name, m_startTimepoint, end, threadID});

		m_stopped = true;
	}


};


#define ND_PROFILE 1
#if ND_PROFILE
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define ND_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define ND_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define ND_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define ND_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define ND_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define ND_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define ND_FUNC_SIG __func__
#else
#define ND_FUNC_SIG "HZ_FUNC_SIG unknown!"
#endif


#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#define ND_PROFILE_BEGIN_SESSION(name, filepath)  ::Scoper::get().beginSession(name, filepath)
#define ND_PROFILE_END_SESSION() ::Scoper::get().endSession()
#define ND_PROFILE_SCOPE(name) ScoperTimer TOKENPASTE2(scoperTim, __LINE__)(name);
#define ND_PROFILE_CALL(call) {ND_PROFILE_SCOPE(#call);call;}
#define ND_PROFILE_METHOD() ND_PROFILE_SCOPE(ND_FUNC_SIG)
#else
#define ND_PROFILE_BEGIN_SESSION(name, filepath)
#define ND_PROFILE_END_SESSION()
#define ND_PROFILE_SCOPE(name)
#define ND_PROFILE_CALL(call) call;
#define ND_PROFILE_METHOD()
#endif
