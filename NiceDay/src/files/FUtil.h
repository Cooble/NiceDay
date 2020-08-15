#include "ndpch.h"
namespace FUtil
{
	inline bool exists(const std::string& path) { return std::filesystem::exists(ND_RESLOC(path)); }
	// folder path of current executable
	const std::string& getExecutableFolderPath();
	// executable path
	const std::string& getExecutablePath();
	
	// absolute path of fileName in the same folder as executable
	// or "" if filename doesn't exist
	std::string getAbsolutePath(const char* fileName);

	void cleanPathString(std::string& s);
	inline std::string cleanPathStringConst(std::string_view s)
	{
		auto out = std::string(s); cleanPathString(out); return out;
	}


	inline void removeSuffix(std::string& s)
	{
		if (s.find_last_of('.') != std::string::npos)
			s = s.substr(0, s.find_last_of('.'));
	}
	inline std::string removeSuffixConst(std::string_view s)
	{
		auto out = std::string(s);
		removeSuffix(out);
		return out;
	}
}