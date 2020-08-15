#include "FUtil.h"
#include <codecvt>
//current implementation works only on windows

static std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}
void FUtil::cleanPathString(std::string& s)
{
	SUtil::replaceWith(s, '\\', '/');
	SUtil::replaceWith(s, "//", "/", 2);
}



std::string FUtil::getAbsolutePath(const char* fileName)
{
	std::string out = getExecutableFolderPath() + fileName;
	return std::filesystem::exists(out) ? out : "";
}

const std::string& FUtil::getExecutableFolderPath()
{
	static std::string out;
	if (out.empty()) {
		out = getExecutablePath().substr(0, getExecutablePath().find_last_of('/')+1);
	}
	return out;
}

const std::string& FUtil::getExecutablePath()
{
	static std::string out;
	if (out.empty()) {
		WCHAR path[260];
		GetModuleFileNameW(NULL, path, 260);
		out = ws2s(std::wstring(path));
		cleanPathString(out);
	}
	return out;
}

