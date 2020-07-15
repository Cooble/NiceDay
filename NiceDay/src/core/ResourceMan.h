#pragma once
#include "ndpch.h"

class ResourceMan
{
	static std::string s_resPath;
public:
	static std::string getResourceLoc(const std::string& resPath);
	static void init();
};
