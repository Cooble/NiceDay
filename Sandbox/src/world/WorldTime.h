﻿#pragma once

enum class Day : int
{
	MON = 0, TUE, WED, THR, FRI, SAT, SUN
};
constexpr long TICKS_PER_MINUTE = 60;
struct WorldTime
{
	long long m_ticks;
	WorldTime(long long ticks) :m_ticks(ticks) {}

	//total hours
	inline float hours() const
	{
		return minutes() / 60;
	}
	//total minutes
	inline float minutes() const
	{
		return (float)m_ticks / TICKS_PER_MINUTE;
	}

	//0-23 hours
	inline float hour() const
	{
		return (float)(m_ticks % (TICKS_PER_MINUTE * 60 * 24)) / TICKS_PER_MINUTE / 60;
	}
	//0-59 mins
	inline float minute() const
	{
		return (float)(m_ticks % (TICKS_PER_MINUTE * 60)) / TICKS_PER_MINUTE;
	}
	inline Day dayOfWeek() const
	{
		return Day((m_ticks % (TICKS_PER_MINUTE * 60 * 24 * 7)) / (TICKS_PER_MINUTE * 60 * 24));
	}

	inline bool isNight() const
	{
		auto hou = hour();
		return hou > 19.5 || hou < 5.5;
	}
	auto ticks()const { return m_ticks; }
	inline long long operator()() const { return m_ticks; }
	inline WorldTime operator+(const WorldTime& t) const
	{
		return t.m_ticks + m_ticks;
	}
	inline WorldTime operator-(const WorldTime& t) const
	{
		return m_ticks - t.m_ticks;
	}
	inline void operator+=(const WorldTime& t)
	{
		m_ticks += t.m_ticks;
	}
	inline void operator-=(const WorldTime& t)
	{
		m_ticks -= t.m_ticks;
	}
};
