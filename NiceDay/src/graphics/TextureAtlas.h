#pragma once

class TextureAtlas
{
private:
	std::unordered_map<std::string, half_int> m_subtextures;
	int m_segmentCount;
public:
	// will search through every image in folderPath and subsequent folders and build an atlas
	void createAtlas(const std::string& folder, int segmentCount, int segmentSize);

	inline int getSize() const { return m_segmentCount; }
	half_int getTexture(const std::string& fileName, const std::string& subName = "main") const;
};
