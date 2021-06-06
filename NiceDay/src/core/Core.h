#pragma once
#include "ndpch.h"
#include <utility>
#include "ResourceMan.h"
#include <type_traits>

namespace nd {
template <class T>
using Ref = std::shared_ptr<T>;

template <class _Ty, class... _Types>
Ref<_Ty> MakeRef(_Types&&... _Args)
{
	// make a shared_ptr
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
#define CLEAR_BIT_WITH_MASK(val,mask)\
	(val)&(~(mask))
#define SET_BIT_WITH_MASK(val,mask)\
	(val)|(mask))

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
		return std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}
};

inline TimerStaper::TimerStaper(std::string&& name) : namee(std::move(name)),
                                                      stop(false)
{
	start = std::chrono::high_resolution_clock::now();
}

inline void TimerStaper::time(const std::string& s)
{
	stop = true;
	long long micros = std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now() - start).count();
	ND_TRACE("[{}] {} It took {}ms", namee.c_str(), s.c_str(), micros / 1000.f);
}

inline long long TimerStaper::getUS()
{
	stop = true;
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).
		count();
}

inline TimerStaper::~TimerStaper()
{
	if (!stop)
		time();
}

#define ND_RESLOC(x)\
	::nd::ResourceMan::getResourceLoc(x)


#define ND_HAS_MEMBER_METHOD_PREPARE(methName)\
template<typename T, typename U = void>\
struct Has_##methName :std::false_type{};\
\
template<typename T>\
struct Has_##methName <T,decltype(/*std::is_member_function_pointer<decltype(*/&T::##methName/*)>::value*/,void())>\
:std::is_member_function_pointer<decltype(&T::##methName)>{};

#define ND_HAS_MEMBER_METHOD(Type,methName)\
	Has_##methName <Type>::value
}
