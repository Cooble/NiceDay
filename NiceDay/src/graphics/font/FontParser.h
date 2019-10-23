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
struct Font
{
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
	std::vector<Character> chars;


	const Character& getChar(int id) const
	{
		for (auto & char_ : chars)
		{
			if (char_.id == id)
				return char_;
		}
		ND_WARN("Invalid font character id request");
		return chars[0];
	}
	int getTextWidth(const std::string& text) const
	{
		int out = 0;
		for (auto c : text)
			out += getChar(c).xadvance;
		return out;
	}
private:
	void bakeUVChars();
};
class FontParser
{
	
public:
	// parse font from file, returns false if parsing failed
	static bool parse(Font& font,const std::string& filePath);

	

	

	
};

