#include "SUtil.h"

/*bool SUtil::replaceWith(Stringo& src, const char* what, const char* with)
{
	Stringo out;
	size_t whatlen = strlen(what);
	out.reserve(src.size());
	size_t ind = 0;
	size_t lastInd = 0;
	while (true)
	{
		ind = src.find(what, ind);
		if (ind == Stringo::npos) {
			out += src.substr(lastInd);
			break;
		}
		out += src.substr(lastInd, ind- lastInd) + with;
		ind += whatlen;
		lastInd = ind;
	}
	src = out;
	return true;
}*/