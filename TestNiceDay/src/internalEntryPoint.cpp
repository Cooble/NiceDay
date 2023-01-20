#include "ndpch.h"
#include "NDTests.h"

// Do not modify this file! (for internal use only)
// declare the method test somehere else
// to use tests wrap your calls in NDT_TRY(_RET) and use NDT_ASSERTs
//
// 
// FOLLOWING FUNCTION needs to be declared in this project!
using namespace nd;
extern void registerTests();

namespace NDT {
    std::vector<Func> funcs;
    void registerTestFunction(const std::string& name, TestFunction t)
    {
        Func f;
        f.name = name;
        f.callback = t;
    
        funcs.push_back(f);
    }
}

int main(int argc,char ** args) noexcept{

    Log::init();	
    registerTests();
    try
    {
    	//run all
    	if(argc==1)
    	{
            ND_WARN("!! Running all tests in one go, not recommended");
            for (auto& func : NDT::funcs) {
                ND_INFO(" => Running test: {}", func.name);
                NDT_TRY(func.callback());
            }
    	}
    	else//run only specified one
    	{
            const char* arg = args[1];
            bool suk = false;
            for (auto& func : NDT::funcs)
            {
	            if(func.name==arg)
	            {
                    suk = true;
                    ND_INFO(" => Running test: {}", func.name);
                    NDT_TRY(func.callback());
                    break;
	            }
            }
            if (!suk) {
                ND_WARN("\n=======:(==========! {} test not found!=======:(==========!");
                return 1;
            }
    	}
		
    }
    catch (...)
    {
        ND_ERROR("=====:(:(:(:(:(======= !Test failed without Assert Error!: ({}):(:(:(:(:(=========", NDT::errorStack().size());
        for (auto& err : NDT::errorStack())
            ND_INFO(err);
        return 1;
    }

	if(NDT::errorExists())
	{
        ND_ERROR("\n\n\n=======:(========== Tests failed with following errors:   ({})==================\n\n\n" ,NDT::errorStack().size());
        int i = 0;
        for (auto& err : NDT::errorStack())
            ND_ERROR("-{}.\t{}", i++, err);

        ND_ERROR("\n\n");
        return 1;
	}
    return 0;
}

