#pragma once
#include "ndpch.h"
#include <optional>

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
		auto textt=Font::removeColorEntities(text);
		int out = 0;
		char lastC = 0;
		for (char c : textt)
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


	static constexpr uint32_t colorToInt(const glm::vec4& vec)
	{
		return
			(
			((uint8_t)(255.0f * vec.r) << 24) |
				((uint8_t)(255.0f * vec.g) << 16) |
				((uint8_t)(255.0f * vec.b) << 8) |
				((uint8_t)(255.0f * vec.a) << 0)
				) & 0xffffffff;
	}

	static std::string colorize(const char* borderColor,const char* color)
	{
		return BORDER_PREFIX+std::string(borderColor)+color;
	}
	static std::string colorizeBorder(const char* borderColor)
	{
		return BORDER_PREFIX + std::string(borderColor);
	}

	
	inline static const char*	DARK_RED=	"&0";
	inline static const char*	RED=		"&1";
	inline static const char*	GOLD=		"&2";
	inline static const char*	YELLOW=		"&3";
	inline static const char*	DARK_GREEN=	"&4";
	inline static const char*	GREEN=		"&5";
	inline static const char*	AQUA=		"&6";
	inline static const char*	DARK_AQUA=	"&7";
	inline static const char*	DARK_BLUE=	"&8";
	inline static const char*	BLUE=		"&9";
	inline static const char*	LIGHT_PURPLE = "&a";
	inline static const char*	DARK_PURPLE="&b" ;
	inline static const char*	WHITE=		"&c" ;
	inline static const char*	GREY=		"&d" ;
	inline static const char*	DARK_GREY=	"&e" ;
	inline static const char*	BLACK=		"&f" ;

	//the color after prefix is borderColor
	inline static const char	BORDER_PREFIX='b';
	//the color after prefix will be print normally (no color conversion)
	inline static const char	IGNORE_PREFIX='i';
	

	inline const static uint32_t colorTemplates[16]
	{
		//0 dark red
		0xbe000000,
		//1 red
		0xfe3f3f00,
		//2 gold
		0xd9a33400,
		//3 yellow
		0xfefe3f00,
		//4 dark green
		0x00be0000,
		//5 green
		0x3ffe3f00,
		//6 aqua
		0x3ffefe00,
		//7 dark aqua
		0x00bebe00,
		//8 dark blue
		0x0000be00,
		//9 blue
		0x3f3ffe00,
		//a light purple
		0xfe3ffe00,
		//b dark purple
		0xbe00be00,
		//c white
		0xffffff00,
		//d grey
		0xbebebe00,
		//e dark grey
		0x3f3f3f00,
		//f black
		0x00000000,
	};

	/**
	 * all entities are 2 or 7 chars in size
	 * & is 2 format: &<0-f>
	 * # is 7 format: #rrggbb
	 */
	static uint32_t entityToColor(const std::string& s);

	/**
	 * all entities are 2 or 7 chars in size
	 * & is 2 format: &<0-f>
	 * # is 7 format: #rrggbb
	 */
	static std::optional<uint32_t> tryEntityToColor(const std::string& s);

	static std::string removeColorEntities(const std::string& s);
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
