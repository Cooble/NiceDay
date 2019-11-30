#pragma once
#include "ndpch.h"
#include <utility>

//#ifdef ND_DEBUG
#define ASSERT(cond,...) if(!(cond))\
	{ND_ERROR("Assertion Failed: {0}",__VA_ARGS__);\
	__debugbreak();}
//#else
//#define ASSERT(cond,message) 
//#endif
#define BIT(x) (1 << (x))

#define SET_BIT(val, index, value)\
	(value?val | BIT(index):val & ~BIT(index))

class TimerStaper
{
private:
	std::chrono::high_resolution_clock::time_point start;
	std::string namee;
	bool stop;
public:
	TimerStaper(std::string name);

	inline void time(const std::string& s = "");
	inline long long getUS();

	~TimerStaper();
};

inline TimerStaper::TimerStaper(std::string name): namee(std::move(name)),
                                       stop(false)
{
	start = std::chrono::high_resolution_clock::now();
}

inline void TimerStaper::time(const std::string& s)
{
	stop = true;
	long long micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
	ND_TRACE("[{}] {} It took {}ms", namee.c_str(), s.c_str(),micros/1000.f);
	
}

inline long long TimerStaper::getUS()
{
	stop = true;
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
}


inline TimerStaper::~TimerStaper()
{
	if (!stop)
		time();
}
#define ND_RESLOC(x)\
	ResourceMan::getResourceLoc(x)
class ResourceMan
{
public:
	static std::string getResourceLoc(const std::string& resPath);
	//static const char* getResourceLoc(const char* resPath);
};
inline std::string ResourceMan::getResourceLoc(const std::string& resPath)
{
	if (resPath._Starts_with("res") || resPath._Starts_with("/res"))
	{
		auto currentDir = std::filesystem::current_path();
		currentDir = currentDir.parent_path();
		return currentDir.string()+"/"+resPath;
	}
	return resPath;
}
/*inline const char* ResourceMan::getResourceLoc(const char* resPath)
{
	std::string resPathh = resPath;
	if (resPathh._Starts_with("res") || resPathh._Starts_with("/res"))
	{
		auto currentDir = std::filesystem::current_path();
		currentDir = currentDir.parent_path();
		return (currentDir.string() + "/" + resPathh).c_str();
	}
	return resPath;
}*/


