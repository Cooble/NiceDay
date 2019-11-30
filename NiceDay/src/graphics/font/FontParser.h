#pragma once
#include "ndpch.h"

struct Character
{
	int id;
	float u, v;
	float u1, v1;
	int xoffset, yoffset;
	int width, height;
	int xadvance;
};

class FontParser;

typedef int FontID;

struct Font
{
	FontID id;
	friend FontParser;

	std::string face;
	std::string texturePath;
	int size;
	bool bold;
	bool italic;
	int padding[4];
	int spacing[2];
	int lineHeight;
	int scaleW, scaleH;
	int base;
	float xSpace = 0, ySpace = 0;
	float xThickness = 0.5, yThickness = 0;
	std::vector<Character> chars;
	std::unordered_map<int, int> kerning;

	inline int getKerning(int firstIndex, int secondIndex) const
	{
		//todo kerning search is probably slow
		auto t = kerning.find(half_int(firstIndex, secondIndex));
		if (t == kerning.end())
			return 0;
		return t->second;
	}

	inline void setKerning(int firstIndex, int secondIndex, int amount)
	{
		kerning[half_int(firstIndex, secondIndex)] = amount;
	}

	const Character& getChar(int id) const
	{
		for (auto& char_ : chars)
		{
			if (char_.id == id)
				return char_;
		}
		//ND_WARN("Invalid font character id request");
		return chars[0];
	}

	int getTextWidth(const std::string& text) const
	{
		int out = 0;
		char lastC = 0;
		for (char c : text)
		{
			out += getChar(c).xadvance + xSpace + getKerning(lastC, c);
			lastC = c;
		}
		return out;
	}

	//wont work on textures that are not square
	inline float getPixelRatio() const
	{
		return 1.f / scaleW;
	}

private:
	void bakeUVChars();
};

#define ND_FONT_LIB_MAX_SIZE 10

class FontLib
{
private:
	inline static std::unordered_map<std::string,Font> s_fonts;
public:
	inline static void registerFont(const std::string& id, const Font& font)
	{
		s_fonts[id] = font;
	}

	inline static const Font* getFont(const std::string& id)
	{
		auto df = s_fonts.find(id);
		if (df == s_fonts.end())
			return nullptr;
		return &s_fonts[id];
	}
};

class FontParser
{
public:
	// parse font from file, returns false if parsing failed
	static bool parse(Font& font, const std::string& filePath);
};
