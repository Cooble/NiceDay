#include "ndpch.h"
#include "TextBuilder.h"
#include "FontParser.h"
#include <optional>

TextMesh::TextMesh(int characterCount)
	: charCount(characterCount) //char_count * vertex_count * x+y * pos+uv 
{
	if (charCount)
		src = new CharM[charCount];
	else src = nullptr;
}

TextMesh::~TextMesh()
{
	if (src)
		delete[] src;
}


void TextMesh::setChar(int index, float x, float y, float x1, float y1, uint32_t color, uint32_t borderColor,
                       const Character& ch)
{
	auto& che = src[index];

	che[0] = {x, y, ch.u, ch.v, color, borderColor};
	che[1] = {x1, y, ch.u1, ch.v, color, borderColor};
	che[2] = {x1, y1, ch.u1, ch.v1, color, borderColor};
	che[3] = {x, y1, ch.u, ch.v1, color, borderColor};
}

void TextMesh::setChar(int index, float x, float y, float x1, float y1, float u, float v, float u1, float v1,
                       uint32_t color, uint32_t borderColor, const Character& ch)
{
	auto& che = src[index];

	che[0] = {x, y, ch.u + u, ch.v + v, color, borderColor};
	che[1] = {x1, y, ch.u1 + u1, ch.v + v, color, borderColor};
	che[2] = {x1, y1, ch.u1 + u1, ch.v1 + v1, color, borderColor};
	che[3] = {x, y1, ch.u + u, ch.v1 + v1, color, borderColor};
}

void TextMesh::resize(int size)
{
	if (src)
		delete[] src;
	charCount = size;

	src = new CharM[charCount];
	currentCharCount = 0;
}

void TextMesh::reserve(int size)
{
	if (this->getMaxCharCount() < size)
		resize(size);
}



struct clipper
{
	float minX, minY, maxX, maxY;
};

static void setCursorData(float currentX, float yLoc, float pixelRat, clipper& clip, const Font& font,
                          CursorProp* cursor)
{
	float x0 = currentX + font.getChar(cursor->cursorCharacter).xoffset;

	auto& cc = font.getChar(cursor->cursorCharacter);
	x0 -= cc.width;

	float y0 = yLoc + cc.yoffset;

	float x1 = x0 + cc.width;
	float y1 = y0 + cc.height;

	float fx0 = std::max(clip.minX, x0);
	float u0 = pixelRat * (fx0 - x0);

	float fx1 = std::min(clip.maxX, x1);
	float u1 = -pixelRat * (x1 - fx1);

	float fy0 = std::max(clip.minY, y0);
	float v0 = pixelRat * (fy0 - y0);

	float fy1 = std::min(clip.maxY, y1);
	float v1 = -pixelRat * (y1 - fy1);
	cursor->positions = {fx0, fy0, fx1, fy1};
	cursor->uvs = {u0, v0, u1, v1};
}


bool TextBuilder::buildMesh(const std::vector<std::u32string>& lines, const Font& font, TextMesh& mesh, int alignment,
	glm::vec<4, int> clipRect, CursorProp* cursor)
{
	clipper clip = { (float)clipRect.x, (float)clipRect.y, (float)clipRect.z, (float)clipRect.w };

	float pixelRat = font.getPixelRatio();


	float yLoc = 0;
	mesh.currentCharCount = 0;
	int currentCharPos = 0;

	float defaultXPos;

	uint32_t color = 0xffffff00;
	uint32_t borderColor = Font::colorToInt({ 0, 0.1, 0.7, 1 });

	for (auto& line : lines)
	{
		float currentX;
		switch (alignment)
		{
		case ALIGN_CENTER:
			currentX = -(float)font.getTextWidth(line) / 2;
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
		int lastC = 0;
		int ignoreColorChar = 0;
		int currentLineIndex = 0;
		for (int c : line)
		{
			bool isBorderPrefix = false;
			bool isIgnorePrefix = false;
			if (currentLineIndex != 0)
			{
				isBorderPrefix = line[currentLineIndex - 1] == Font::BORDER_PREFIX;
				isIgnorePrefix = line[currentLineIndex - 1] == Font::IGNORE_PREFIX;
			}

			if (c == '&')
			{
				auto col = Font::tryEntityToColor(line.substr(currentLineIndex));
				if (col.has_value() && !isIgnorePrefix)
				{
					if (!isBorderPrefix)
						color = col.value();
					else
						borderColor = col.value();
					ignoreColorChar = 2;
				}
			}
			else if (c == '#')
			{
				auto col = Font::tryEntityToColor(line.substr(currentLineIndex));
				if (col.has_value() && !isIgnorePrefix)
				{
					if (!isBorderPrefix)
						color = col.value();
					else
						borderColor = col.value();
					ignoreColorChar = 7;
				}

			}
			else if ((c == Font::BORDER_PREFIX || c == Font::IGNORE_PREFIX) && currentLineIndex + 1 < line.size())//check if in future the color is valid
			{
				auto col = Font::tryEntityToColor(line.substr(currentLineIndex + 1));
				if (col.has_value())
				{
					ignoreColorChar = 1;
				}
			}

			currentLineIndex++;
			if (ignoreColorChar)
			{
				ignoreColorChar--;
				currentCharPos++;
				continue;
			}
			float kerning = font.getKerning(lastC, c);
			if (mesh.currentCharCount == mesh.getMaxCharCount())
			{
				ND_WARN("TextMesh too small");
				return false;
			}
			auto& cc = font.getChar(c);

			float x0 = currentX + cc.xoffset + kerning;
			float y0 = yLoc + cc.yoffset;

			float x1 = x0 + cc.width;
			float y1 = y0 + cc.height;

			if (!(x1 < clip.minX || x0 > clip.maxX || y1 < clip.minY || y0 > clip.maxY))
			{
				float fx0 = std::max(clip.minX, x0);
				float u0 = pixelRat * (fx0 - x0);

				float fx1 = std::min(clip.maxX, x1);
				float u1 = -pixelRat * (x1 - fx1);

				float fy0 = std::max(clip.minY, y0);
				float v0 = pixelRat * (fy0 - y0);

				float fy1 = std::min(clip.maxY, y1);
				float v1 = -pixelRat * (y1 - fy1);

				mesh.setChar(mesh.currentCharCount++, fx0, fy0, fx1, fy1, u0, v0, u1, v1, color, borderColor, cc);

				if (cursor != nullptr && currentCharPos == cursor->cursorPos)
				{
					setCursorData(currentX, yLoc, pixelRat, clip, font, cursor);
					cursor = nullptr;
				}
			}

			currentX += cc.xadvance + font.xSpace + kerning;
			currentCharPos++;
			lastC = c;
		}
		//on the edge of line
		if (cursor && currentCharPos == cursor->cursorPos)
		{
			setCursorData(currentX, yLoc, pixelRat, clip, font, cursor);
			cursor = nullptr;
		}
		yLoc -= font.lineHeight + font.ySpace;
	}

	return true;
}


void TextBuilder::convertToLines(const std::string& text, const Font& font, int maxLineWidth,
	std::vector<std::u32string>& lines)
{
	std::vector<std::string> separatedLines;
	SUtil::splitString(Font::removeColorEntities(text), separatedLines, "\n");

	for (auto& sepLine : separatedLines)
	{
		int currentWidth = 0;
		std::string currentLine;
		std::vector<std::string> words;
		SUtil::splitString(sepLine, words);
		for (auto& word : words)
		{
			auto wordSize = font.getTextWidth(word);
			if (currentWidth + wordSize <= maxLineWidth)
			{
				currentLine += " " + word;
			}
			else
			{
				lines.push_back(SUtil::utf8toCodePoints(currentLine));
				currentWidth = wordSize;
				currentLine = word;
			}
		}
		if (!currentLine.empty())
			lines.push_back(SUtil::utf8toCodePoints(currentLine));
	}
}