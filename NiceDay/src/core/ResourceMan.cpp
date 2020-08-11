#include "ResourceMan.h"
#include "files/FUtil.h"
std::string ResourceMan::s_resPath;
std::string ResourceMan::s_resPathFolder;

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
			s_resPathFolder = s_resPath + "res";
			ND_TRACE("Res folder found: {}res", s_resPath.c_str());
			return;
		}
	}
	ND_ERROR("Res folder not found");
}

const std::string& ResourceMan::getResPath()
{
	return s_resPathFolder;
}

std::string ResourceMan::getResourceLoc(const std::string& resPath)
{

	if (SUtil::startsWith(resPath, "res") || SUtil::startsWith(resPath, "/res"))
		return s_resPath + resPath;
	return resPath;
}

std::string ResourceMan::getLocalPath(const std::string& resPath)
{
	auto res = resPath;
	FUtil::cleanPathString(res);
	size_t  offset = 0;
	size_t out = std::string::npos;
	while(true)
	{
		auto index = res.find("/res/",offset);
		if(index==std::string::npos)
			break;
		offset = index + 1;
		out = index;
	}
	if (out == std::string::npos)
		return "";
	return res.substr(out+1);
}
