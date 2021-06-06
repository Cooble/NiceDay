#pragma once
#pragma warning(disable : 26812)
#include "ndpch.h"

namespace nd {

typedef int TextureAtlasFlags;

enum TextureAtlasFlags_
{
	TextureAtlasFlags_None = 0,
	TextureAtlasFlags_FlipY = 1 << 0,
	// if size is too small returns false instead of resizing
	// atlas is then invalid, returns false
	TextureAtlasFlags_DontResizeIfNeccessary = 1 << 1,
	// creates atlas.png in the folder (useful for debugging)
	TextureAtlasFlags_CreateFile = 1 << 2,
	// will not create graphical object texture (used possibly when TextureAtlasFlags_CreateFile)
	// texture remains nullptr
	TextureAtlasFlags_DontCreateTexture = 1 << 3,
};

class Texture;

class TextureAtlas
{
private:
	Texture* m_texture = nullptr;
	std::unordered_map<Strid, half_int> m_subtextures;
	int m_segmentCount;
public:
	~TextureAtlas();
	// will search through every image in folderPath and subsequent folders and build an atlas
	// NOTE: folder must not end with '/' !
	bool createAtlas(std::string_view folder, int segmentCount, int segmentSize, TextureAtlasFlags flags);

	int getSize() const { return m_segmentCount; }
	half_int getSubImage(const std::string& fileName, const char* subName = "main") const;
	const Texture* getTexture() const { return m_texture; }
};

struct TextureAtlasUVCoords
{
	glm::vec2 min;
	glm::vec2 max;
	glm::ivec2 pixelSize;

	constexpr glm::vec2 size() const { return max - min; }
};

class TextureAtlasUV
{
private:
	Texture* m_texture = nullptr;
	int m_size;
	std::unordered_map<Strid, TextureAtlasUVCoords> m_subtextures;
public:
	~TextureAtlasUV();
	// will search through every image in folderPath and subsequent folders and build an atlas
	// returns true on success
	// NOTE: folder must not end with '/' !
	bool createAtlas(std::string_view folder, int size, int padding = 0, TextureAtlasFlags flags = 0);

	const TextureAtlasUVCoords& getSubImage(StringId fileName) const { return m_subtextures.at(fileName.getValue()); }
	const Texture* getTexture() const { return m_texture; }
};
}
