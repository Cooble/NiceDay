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
	std::chrono::system_clock::time_point start;
	std::string namee;
	bool stop;
public:
	TimerStaper(std::string name);

	inline void time(const std::string& s = "");

	~TimerStaper();
};

inline TimerStaper::TimerStaper(std::string name): namee(std::move(name)),
                                       stop(false)
{
	start = std::chrono::system_clock::now();
}

inline void TimerStaper::time(const std::string& s)
{
	stop = true;
	long millis = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count();
	ND_TRACE("[{}] {} It took {}ms", namee.c_str(), s.c_str(),millis/1000.0f);
}

inline TimerStaper::~TimerStaper()
{
	if (!stop)
		time();
}
