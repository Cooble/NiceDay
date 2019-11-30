#include "ndpch.h"
#include "core/AppGlobals.h"

AppGlobals& AppGlobals::get()
{
	static AppGlobals g;
	return g;
}
