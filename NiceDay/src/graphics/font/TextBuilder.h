#pragma once
#include "FontParser.h"

struct Font;

class TextMesh
{
private:
	int charCount;
public:
	struct Vertex
	{
		union {

			struct { float x, y; };
			glm::vec2 pos;
		};
		union {

			struct { float u,v; };
			glm::vec2 uv;
		};
		uint32_t color;
		uint32_t borderColor;
	};
	struct CharM
	{
		Vertex vertex[4];
		Vertex& operator[](int index) { return vertex[index]; }
		const Vertex& operator[](int index) const { return vertex[index]; }
	};

	
	
	CharM* src;
	// usually stores hash of the string
	Strid id;
	
public:
	//how many chars are active
	//range from 0 to getCharCount()
	int currentCharCount=0;
	TextMesh(int characterCount=0);

	~TextMesh();

	int getMaxCharCount() const { return charCount; }
	int getMaxByteSize() const { return charCount*sizeof(CharM); }
	CharM* getSrc() { return src; }
	const CharM* getSrc() const { return src; }
	void clear() { memset(src, 0, sizeof(CharM) * charCount); }
	void setChar(int index, float x, float y, float x1, float y1,uint32_t color,uint32_t borderColor, const Character& ch);

	void setChar(int index, float x, float y, float x1, float y1,float u,float v,float u1,float v1 , uint32_t color, uint32_t borderColor, const Character& ch);
	
	int getVertexCount() const
	{
		return currentCharCount * 4;
	}
	//sets this->size to match size
	void resize(int size);
	//ensures that this->size is at least param size
	void reserve(int size);
};

struct CursorProp
{
	int cursorPos=-1;
	char cursorCharacter;

	glm::vec4 positions;
	glm::vec4 uvs;
	
	bool isEnabled()const { return cursorPos != -1; }
};

class TextBuilder
{

public:
	enum :int
	{
		ALIGN_RIGHT,
		ALIGN_LEFT,
		ALIGN_CENTER,
	};
	// converts utf-8 encoded text to lines with pixel maxWidth encoded as pure codePoints
	static void convertToLines(const std::string& text, const Font& font, int maxLineWidth, std::vector<std::u32string>& lines);

	//updates mesh with data
	//return false if mesh is too small to fit all chars in
	//clipRect {minX,minY,maxX,maxY}
	static bool buildMesh(const std::vector<std::u32string>& lines, const Font& font, TextMesh& mesh, int alignment = ALIGN_LEFT,
		glm::vec<4, int> clipRect = { -10000,-10000,20000,20000 }, CursorProp* cursor = nullptr);
	static bool buildMesh(const std::string& text, int maxLineWidth, const Font& font, TextMesh& mesh, int alignment = ALIGN_LEFT,
		glm::vec<4, int> clipRect = { -10000,-10000,20000,20000 }, CursorProp* cursor = nullptr)
	{
		std::vector<std::u32string> lines;
		convertToLines(text, font, maxLineWidth, lines);
		return buildMesh(lines, font, mesh, alignment, clipRect, cursor);
	}

};
