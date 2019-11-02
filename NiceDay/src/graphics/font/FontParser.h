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
			out += getChar(c).xadvance+xSpace;
		return out;
	}
private:
	void bakeUVChars();
};

#define ND_FONT_LIB_MAX_SIZE 10
class FontLib
{
private:
	inline static std::array<Font,ND_FONT_LIB_MAX_SIZE> s_fonts;
	static int s_current_index;
public:
	inline static FontID registerFont(const Font& font)
	{
		ASSERT(s_current_index < s_fonts.max_size(), "ND_FONT_LIB_MAX_SIZE too low");
		s_fonts[s_current_index++]=font;
		FontID id = s_current_index-1;
		s_fonts[id].id= id;
		return id;
	}
	inline static const Font* getFont(FontID id)
	{
		ASSERT(id < s_current_index, "Invalid FontID");
		return &s_fonts[id];
	}
	inline static void clear() { s_current_index=0; }
	
};

class FontParser
{
	
public:
	// parse font from file, returns false if parsing failed
	static bool parse(Font& font,const std::string& filePath);

	

	

	
};

