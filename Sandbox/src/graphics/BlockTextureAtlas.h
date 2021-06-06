#pragma once
#include "ndpch.h"
class BlockTextureAtlas
{
private:
	std::unordered_map<std::string, half_int> m_subtextures;
public:
	// will search through every image in folderPath and subsequent folders and build an atlas
	void createAtlas(const std::string& folder, int segmentCount, int segmentSize);

	half_int getTexture(const std::string& fileName, const std::string& subName = "main") const;
};
