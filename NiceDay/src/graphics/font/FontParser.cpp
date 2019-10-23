#include "ndpch.h"
#include "FontParser.h"

void Font::bakeUVChars()
{
	//flips Y axis and calculates UV
	for (auto& ch : chars)
	{
		ch.u1 = (ch.u +ch.width)/scaleW;
		ch.v1 = (ch.v +ch.height)/scaleH;

		ch.u /= scaleW;
		ch.v /= scaleH;

		ch.v1 = 1 - ch.v1;
		ch.v = 1 - ch.v;
		auto foo = ch.v;
		ch.v = ch.v1;
		ch.v1 = foo;

		ch.yoffset =lineHeight-ch.yoffset-ch.height;
	}
}

bool FontParser::parse(Font& font,const std::string& filePath)
{
	font = {};

	int charCount = 0;
	auto stream = std::ifstream(ND_RESLOC(filePath));
	if (!stream.is_open()) {
#ifdef ND_DEBUG
		ND_WARN("Invalid font path: {}", filePath);
#endif
		return false;
	}
	std::string line;
	std::vector<std::string> words;
	while(!stream.eof())
	{
		std::getline(stream, line);
		words.clear();
		if(line.empty())
			continue;;

		NDUtil::splitString(line, words);
		auto& title = words[0];

		if(title == "info")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=')+1);
				if(var._Starts_with("face="))
					font.face = arg.substr(1, arg.size() - 2);
				else if(var._Starts_with("size="))
					font.size = std::stoi(arg);
				else if (var._Starts_with("bold="))
					font.bold = (bool)std::stoi(arg);
				else if (var._Starts_with("italic="))
					font.italic = (bool)std::stoi(arg);
				else if (var._Starts_with("padding="))
				{
					std::vector<std::string> data;
					NDUtil::splitString(arg, data,",");
					ASSERT(data.size() == 4, "Invalid font file");
					for (int i = 0; i < 4; ++i)
						font.padding[i] = std::stoi(data[i]);
				}
				else if (var._Starts_with("spacing="))
				{
					std::vector<std::string> data;
					NDUtil::splitString(arg, data, ",");
					ASSERT(data.size() == 2, "Invalid font file");
					for (int i = 0; i < 2; ++i)
						font.spacing[i] = std::stoi(data[i]);
				}
			}
		}
		else if(title == "common")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=')+1);
				
				if (var._Starts_with("lineHeight="))
					font.lineHeight = std::stoi(arg);
				else if (var._Starts_with("base="))
					font.base = std::stoi(arg);
				else if (var._Starts_with("scaleW="))
					font.scaleW = std::stoi(arg);
				else if (var._Starts_with("scaleH="))
					font.scaleH = std::stoi(arg);
			}
		}
		else if (title == "page")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=')+1);

				if (var._Starts_with("file="))
					font.texturePath = arg.substr(1, arg.size() - 2);;
			}
		}
		else if (title == "chars")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=')+1);

				if (var._Starts_with("count="))
					charCount = std::stoi(arg);
			}
		}
		else if (title == "char")
		{
			Character ch={};
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=')+1);

				if (var._Starts_with("id="))
					ch.id = std::stoi(arg);
				else if (var._Starts_with("x="))
					ch.u = std::stoi(arg);
				else if (var._Starts_with("y="))
					ch.v = std::stoi(arg);
				else if (var._Starts_with("width="))
					ch.width = std::stoi(arg);
				else if (var._Starts_with("height="))
					ch.height = std::stoi(arg);
				else if (var._Starts_with("xoffset="))
					ch.xoffset = std::stoi(arg);
				else if (var._Starts_with("yoffset="))
					ch.yoffset = std::stoi(arg);
				else if (var._Starts_with("xadvance="))
					ch.xadvance = std::stoi(arg);
			}
			font.chars.push_back(ch);
		}
	}
#ifdef ND_DEBUG
	if(font.chars.size() != charCount)
		ND_WARN("Strange font file {}",ND_RESLOC(filePath));
#endif
	font.bakeUVChars();
	return true;
}
