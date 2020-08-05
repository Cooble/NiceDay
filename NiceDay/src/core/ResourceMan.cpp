#include "ResourceMan.h"
#include "files/FUtil.h"
std::string ResourceMan::s_resPath;

void  ResourceMan::init()
{
	auto currentDir = std::filesystem::path(FUtil::getExecutableFolderPath());
	//LOOK 3 STEPS UP for /res
	for (int i = 0; i < 4; ++i)
	{
		currentDir = currentDir.parent_path();
		if (std::filesystem::exists(currentDir.string() + "/res"))
		{
			s_resPath = currentDir.string() + "/";
			ND_TRACE("Res folder found: {}res", s_resPath.c_str());
			return;
		}
	}
	ND_ERROR("Res folder not found");
}
std::string ResourceMan::getResourceLoc(const std::string& resPath)
{

	if (SUtil::startsWith(resPath, "res") || SUtil::startsWith(resPath, "/res"))
		return s_resPath + resPath;
	return resPath;
}