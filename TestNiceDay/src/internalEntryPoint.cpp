#include "ndpch.h"
#include "NDTests.h"

// Do not modify this file! (for internal use only)
// declare the method test somehere else
// to use tests wrap your calls in NDT_TRY(_RET) and use NDT_ASSERTs
//
// 
// FOLLOWING FUNCTION needs to be declared in this project!
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
            std::cout << "\n!! Running all tests in one go, not recommended\n" << std::endl;
	        for (auto& func: NDT::funcs)
                NDT_TRY(func.callback());
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
                    NDT_TRY(func.callback());
                    break;
	            }
            }
            if (!suk) {
                std::cout << "\n=======:(==========! "<<std::string(arg) << " test not found!=======:(==========!" << std::endl;
                return 1;
            }
    	}
		
    }
    catch (...)
    {
        std::cout << "=====:(:(:(:(:(======= !Test failed without Assert Error!:   (" << NDT::errorStack().size() << "):(:(:(:(:(=========\n";
        for (auto& err : NDT::errorStack())
            std::cout << err << std::endl;
        return 1;
    }

	if(NDT::errorExists())
	{
        std::cout << "\n\n\n=======:(========== Tests failed with following errors:   (" << NDT::errorStack().size() << ")==================\n\n";
        int i = 0;
		for (auto& err : NDT::errorStack())
            std::cout<<" -"<<i++<<".\t" << err << std::endl;
        std::cout << "\n\n";
        return 1;

	}
    return 0;
}

