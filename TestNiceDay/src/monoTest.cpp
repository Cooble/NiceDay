#include "ndpch.h"
#include "NDTests.h"
#include "script/MonoLayer.h"

using namespace nd;

int monoTest()
{
	MonoLayer l;
	l.hotSwapEnable = false;
	l.onAttach();
	NDT_ASSERT(l.isMonoLoaded());
	l.onDetach();
    return 0;
}
