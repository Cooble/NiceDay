#include "ndpch.h"
namespace FUtil
{
	// folder path of current executable
	const std::string& getExecutableFolderPath();
	// executable path
	const std::string& getExecutablePath();
	
	// absolute path of fileName in the same folder as executable
	// or "" if filename doesn't exist
	std::string getAbsolutePath(const char* fileName);

	void cleanPathString(std::string& s);
}