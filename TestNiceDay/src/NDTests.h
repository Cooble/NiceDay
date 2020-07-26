#pragma once
#include "string"
#include <vector>

namespace NDT
{
	extern std::vector<std::string> error_stack;

	inline bool errorExists() { return !error_stack.empty(); }
	inline auto& errorStack() { return error_stack; }
	inline void pushError(const std::string& e) { error_stack.push_back(e); }
}

#define NDT_STRINGIFY(x) #x
#define NDT_TOSTRING(x) NDT_STRINGIFY(x)
#define NDT_FILE_LINE NDT_TOSTRING(__FILE__) ":" NDT_TOSTRING(__LINE__)

#define NDT_ASSERT(cond)\
	if(!(cond)){throw std::string("Assert failed at: " NDT_FILE_LINE);}

#define NDT_ASSERT_EQUAL(value,target)\
	if((value)!=(target)){throw std::string("Assert failed at: " NDT_FILE_LINE);}
#define NDT_ASSERT_NOT_EQUAL(value,target)\
	if((value)==(target)){throw std::string("Assert failed at: " NDT_FILE_LINE);}

#define NDT_TRY(call)\
	 try{call;}catch(const std::string& s){NDT::pushError(s);std::cout << s << std::endl;}
#define NDT_TRY_RET(call)\
	 try{call;}catch(const std::string& s){NDT::pushError(s);std::cout << s << std::endl;return 1;}

	
   