#pragma once

//typedef void(*funct)(int);
template <typename T>
struct Caller
{
	void operator()(int idx, T oneParam) const;
};

template <int I,typename T>
inline void looper(T args)
{
	Caller<T>(I, args);

	looper<(I - 1),T>(args);

}

