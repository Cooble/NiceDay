#pragma once
#include "ndpch.h"
#include <utility>
#include "ResourceMan.h"

template<class T>
using Ref = std::shared_ptr<T>;
template <class _Ty, class... _Types>
Ref<_Ty> MakeRef(_Types&&... _Args) { // make a shared_ptr
	return std::make_shared<_Ty>(std::forward<_Types>(_Args)...);
}

//#ifdef ND_DEBUG
#define ASSERT(cond,...) if(!(cond))\
	{ND_ERROR("Assertion Failed: {}",#cond);\
	ND_ERROR(__VA_ARGS__);\
	__debugbreak();}
//#else
//#define ASSERT(cond,message) 
//#endif
#define BIT(x) (1 << (x))

#define SET_BIT(val, index, value)\
	((value)?(val) | BIT(index):(val) & ~BIT(index))

class TimerStaper
{
private:
	std::chrono::high_resolution_clock::time_point start;
	std::string namee;
	bool stop;
public:
	TimerStaper(std::string&& name);

	inline void time(const std::string& s = "");
	inline long long getUS();

	~TimerStaper();

	static uint64_t getNowMicros()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
};

inline TimerStaper::TimerStaper(std::string&& name): namee(std::move(name)),
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




