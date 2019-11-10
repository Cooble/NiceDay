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
	};
	struct CharM
	{
		Vertex vertex[4];
		inline Vertex& operator[](int index) { return vertex[index]; }
		inline const Vertex& operator[](int index) const { return vertex[index]; }
	};

	
	
	CharM* src;
	
public:
	//how many chars are active
	//range from 0 to getCharCount()
	int currentCharCount=0;
	TextMesh(int characterCount=0);

	~TextMesh();

	inline int getMaxCharCount() const { return charCount; }
	inline int getMaxByteSize() const { return charCount*sizeof(CharM); }
	inline CharM* getSrc() { return src; }
	inline const CharM* getSrc() const { return src; }
	inline void clear() { memset(src, 0, sizeof(CharM) * charCount); }
	inline void setChar(int index, float x, float y, float x1, float y1, const Character& ch);
	
	inline int getVertexCount() const
	{
		return currentCharCount * 4;
	}

	void resize(int size);
};

class TextBuilder
{
	
public:	
	enum:int
	{
		ALIGN_RIGHT,		
		ALIGN_LEFT,		
		ALIGN_CENTER,		
	};
	static void convertToLines(const std::string& text, const Font& font, int maxLineWidth, std::vector<std::string>& lines);

	//updates mesh with data
	//return false if mesh is too small to fit all chars in
	static bool buildMesh(const std::vector<std::string>& lines, const Font& font, TextMesh& mesh, int alignment=ALIGN_LEFT);
	static bool buildMesh(const std::string& text, int maxLineWidth, const Font& font, TextMesh& mesh, int alignment=ALIGN_LEFT)
	{
		std::vector<std::string> lines;
		convertToLines(text, font, maxLineWidth, lines);
		return buildMesh(lines, font, mesh, alignment);
	}
	
};
