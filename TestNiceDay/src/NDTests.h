#pragma once
#include "string"
#include <vector>

namespace NDT
{
	extern std::vector<std::string> error_stack;

	inline bool errorExists() { return !error_stack.empty(); }
	inline auto& errorStack() { return error_stack; }
	inline void pushError(const std::string& e) { error_stack.push_back(e); }

	typedef int (*TestFunction)();
	struct Func
	{
		std::string name;
		TestFunction callback;
	};
	void registerTestFunction(const std::string& name, TestFunction t);

}
#ifdef __GNUC__
#define NDT_FUNCTION_NAME __PRETTY_FUNCTION__
#else 
#define NDT_FUNCTION_NAME __FUNCTION__ 
#endif
#define NDT_STRINGIFY(x) #x
#define NDT_TOSTRING(x) NDT_STRINGIFY(x)
#define NDT_FILE_LINE NDT_TOSTRING(__FILE__) ":(" NDT_TOSTRING(__LINE__) ") in " NDT_FUNCTION_NAME

#define NDT_ASSERT(cond)\
	if(!(cond)){throw std::string("Assert failed at: " NDT_FILE_LINE);}

#define NDT_ASSERT_EQUAL(value,target)\
	if((value)!=(target)){throw std::string("Assert failed at: " NDT_FILE_LINE);}
#define NDT_ASSERT_NOT_EQUAL(value,target)\
	if((value)==(target)){throw std::string("Assert failed at: " NDT_FILE_LINE);}

#define NDT_TRY(call)\
	 try{call;}catch(const std::string& s){NDT::pushError(s);/*std::cout << s << std::endl;return 1;*/}
#define NDT_TRY_RET(call)\
	 try{call;}catch(const std::string& s){NDT::pushError(s);/*std::cout << s << std::endl;return 1;*/}

#define NDT_REGISTER_TEST(func)\
	extern int func ();\
	NDT::registerTestFunction(#func,func);

	
   