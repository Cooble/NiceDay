#include "ndpch.h"
#include "AppGlobals.h"

AppGlobals& AppGlobals::get()
{
	static AppGlobals g;
	return g;
}
