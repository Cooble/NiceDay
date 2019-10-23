#pragma once
#include "FontParser.h"

struct Font;

class TextMesh
{
private:
	struct Vertex
	{
		float x, y;
		float u, v;
	};
	struct CharM
	{
		Vertex vertex[6];
		inline Vertex& operator[](int index) { return vertex[index]; }
	};
	
	const int charCount;
	CharM* src;
	
public:
	//how many chars are active
	//range from 0 to getCharCount()
	int currentCharCount=0;
	TextMesh(int characterCount);
	~TextMesh();

	inline int getMaxCharCount() const { return charCount; }
	inline int getByteSize() const { return charCount*sizeof(CharM); }
	inline float* getSrc() { return (float*)src; }
	inline void clear() { memset(src, 0, sizeof(CharM) * charCount); }
	inline void setChar(int index, float x, float y, float x1, float y1, const Character& ch);
	inline int getVertexCount()
	{
		return currentCharCount * 6;
	}
};

class TextBuilder
{
	enum:int
	{
		ALIGN_RIGHT,		
		ALIGN_LEFT,		
		ALIGN_CENTER,		
	};
	
public:	
	static void convertToLines(const std::string& text, const Font& font, int maxLineWidth, std::vector<std::string>& lines);

	//updates mesh with data
	//return false if mesh is too small to fit all chars in
	static bool buildMesh(const std::vector<std::string>& lines, const Font& font, TextMesh& mesh, int alignment=ALIGN_LEFT);
	static bool buildMesh(const std::string& text, const Font& font, TextMesh& mesh, int maxLineWidth, int alignment=ALIGN_LEFT)
	{
		std::vector<std::string> lines;
		convertToLines(text, font, maxLineWidth, lines);
		return buildMesh(lines, font, mesh, alignment);
	}
	
};
