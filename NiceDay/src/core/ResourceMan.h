#pragma once
#include "ndpch.h"

namespace nd {
class ResourceMan
{
	// folder where /res is located
	static std::string s_resPath;
	// res folder
	static std::string s_resPathFolder;
public:
	static std::string getResourceLoc(std::string_view resPath);
	static std::string getLocalPath(std::string_view resPath);
	static void init();
	static const std::string& getResPath();
};
}
