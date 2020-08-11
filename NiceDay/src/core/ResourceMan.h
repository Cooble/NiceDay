#pragma once
#include "ndpch.h"

class ResourceMan
{
	// folder where /res is located
	static std::string s_resPath;
	// res folder
	static std::string s_resPathFolder;
public:
	static std::string getResourceLoc(const std::string& resPath);
	static std::string getLocalPath(const std::string& resPath);
	static void init();
	static const std::string& getResPath();
	
};
