#include "ndpch.h"
#include "core/AppGlobals.h"

nd::AppGlobals& nd::AppGlobals::get()
{
	static AppGlobals g;
	return g;
}
