#pragma once
#ifdef ND_DEBUG
#define ASSERT(cond,...) if(!(cond)){ND_ERROR("Assertion Failed: {0}", __VA_ARGS__);__debugbreak();}
#else
#define ASSERT(cond,message) 
#endif
#define BIT(x) 1 << x