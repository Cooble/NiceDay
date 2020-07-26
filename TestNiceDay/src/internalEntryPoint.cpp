#include "ndpch.h"
#include "NDTests.h"

// Do not modify this file! (for internal use only)
// declare the method test somehere else
// to use tests wrap your calls in NDT_TRY(_RET) and use NDT_ASSERTs
//
// 
// FOLLOWING FUNCTION needs to be declared in this project!
extern int tests(int argc, char** args);

int main(int argc,char ** args) noexcept{

    Log::init();
    ND_INFO("Running tests in exe..");
    int ret = -1;

    try
    {
		ret = tests(argc, args);
    }
    catch (...)
    {
        std::cout << "Tests failed without Assert Error!:   (" << NDT::errorStack().size() << ")\n";
        for (auto& err : NDT::errorStack())
            std::cout << err << std::endl;
        return 1;
    }

	if(NDT::errorExists())
	{
        std::cout << "Tests failed with following errors:   (" << NDT::errorStack().size() << ")\n";
		for (auto& err : NDT::errorStack())
            std::cout << err << std::endl;
        return 1;
	}
    if (ret == 0)
        ND_INFO("Tests succeded");
    return ret;
}

