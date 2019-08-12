#pragma once

class TextureAtlas
{
private:
	std::unordered_map<std::string, half_int> m_subtextures;
public:
	// will search through every image in folderPath and subsequent folders and build an atlas
	void createAtlas(const std::string& folder, int segmentCount, int segmentSize);
	
	inline half_int getTexture(const std::string& fileName, const std::string& subName="main")const
	{
		auto& i = m_subtextures.find(fileName + subName);
		if(i==m_subtextures.end())
		{
			ND_ERROR("Trying to retrieve not loaded texture: {}", (fileName + +":\t\t"+subName));
			return 0;
		}
		return i->second;
	}
};
