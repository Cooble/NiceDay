#include "ndpch.h"
#include "TextBuilder.h"
#include "FontParser.h"

TextMesh::TextMesh(int characterCount)
	: charCount(characterCount) //char_count * vertex_count * x+y * pos+uv 
{
	src = new CharM[charCount];
}

TextMesh::~TextMesh()
{
	if(src)
		delete[] src;
}

void TextMesh::setChar(int index, float x, float y, float x1, float y1, const Character& ch)
{
	auto& che = src[index];

	che[0] = {x, y, ch.u, ch.v};
	che[1] = {x1, y, ch.u1, ch.v};
	che[2] = {x1, y1, ch.u1, ch.v1};
	che[3] = {x, y1, ch.u, ch.v1};

}

void TextBuilder::convertToLines(const std::string& text, const Font& font, int maxLineWidth,
                                 std::vector<std::string>& lines)
{
	std::vector<std::string> separatedLines;
	NDUtil::splitString(text, separatedLines, "\n");

	for (auto& sepLine : separatedLines)
	{
		int currentWidth = 0;
		std::string currentLine;
		std::vector<std::string> words;
		NDUtil::splitString(sepLine, words);
		for (auto& word : words)
		{
			auto wordSize = font.getTextWidth(word);
			if (currentWidth + wordSize <= maxLineWidth)
			{
				currentLine += " " + word;
			}
			else
			{
				lines.push_back(currentLine);
				currentWidth = wordSize;
				currentLine = word;
			}
		}
		if (!currentLine.empty())
			lines.push_back(currentLine);
	}
}

bool TextBuilder::buildMesh(const std::vector<std::string>& lines, const Font& font, TextMesh& mesh, int alignment)
{
	float yLoc = 0;
	mesh.currentCharCount = 0;

	float defaultXPos;

	for (auto& line : lines)
	{
		float currentX;
		switch (alignment)
		{
		case ALIGN_CENTER:
			currentX = -(float)font.getTextWidth(line)/2;
			break;
		case ALIGN_LEFT:
			currentX = 0;
			break;
		case ALIGN_RIGHT:
			currentX = -(float)font.getTextWidth(line);
			break;
		default:
			currentX = 0;
			ASSERT(false, "Invalid ALIGNMENT");
			break;
		}
		for (auto c : line)
		{
			
			if (mesh.currentCharCount == mesh.getMaxCharCount())
			{
				ND_WARN("TextMesh too small");
				return false;
			}
			auto& cc = font.getChar(c);

			mesh.setChar(mesh.currentCharCount,
			             currentX + cc.xoffset, 
				yLoc + cc.yoffset,
			             currentX + cc.xoffset + cc.width, 
				yLoc + cc.yoffset + cc.height, cc);
			currentX += cc.xadvance+font.xSpace;
			mesh.currentCharCount++;
		}
		yLoc -= font.lineHeight+font.ySpace;
	}

	return true;
}

