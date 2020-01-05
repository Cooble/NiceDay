#include "ndpch.h"
#include "FontParser.h"

uint32_t Font::entityToColor(const std::string& s)
{
	const char c = s[0];
	const char cc = s[1];
	switch (c)
	{
	case '&':
		if (s.size() < 2)
			return 0;
		if (cc >= '0' && cc <= '9')
			return colorTemplates[cc - '0'];
		else if (cc >= 'a' && cc <= 'f')
			return colorTemplates[(cc - 'a') + 10];
		else if (cc >= 'A' && cc <= 'F')
			return colorTemplates[(cc - 'A') + 10];
		return 0;
	case '#':
		if (s.size() < 7)
			return 0;
		return (std::stoi(s.substr(1), nullptr, 16) << 8) & 0xffffff00;
	}
	return 0;
}

std::optional<uint32_t> Font::tryEntityToColor(const std::string& s)
{
	const char c = s[0];
	switch (c)
	{
	case '&':
	{
		const char cc = s[1];
		if (s.size() < 2)
			return std::optional<uint32_t>();
		if (cc >= '0' && cc <= '9')
			return colorTemplates[cc - '0'];
		else if (cc >= 'a' && cc <= 'f')
			return colorTemplates[(cc - 'a') + 10];
		else if (cc >= 'A' && cc <= 'F')
			return colorTemplates[(cc - 'A') + 10];
		return std::optional<uint32_t>();
	}
	case '#':
		if (s.size() < 7)
			return std::optional<uint32_t>();
		try
		{
			return (std::stoi(s.substr(1, 6), nullptr, 16) << 8) & 0xffffff00;
		}
		catch (...)
		{
			return std::optional<uint32_t>();
		}
	}
	return std::optional<uint32_t>();
}

std::string Font::removeColorEntities(const std::string& s)
{
	std::string out;
	out.reserve(s.size());
	for (int i = 0; i < s.size(); ++i)
	{
		char c = s[i];
		bool isBorderPrefix = false;
		bool isIgnorePrefix = false;
		if ((c == '&' || c == '#') && i != 0)
		{
			isBorderPrefix = s[i - 1] == Font::BORDER_PREFIX;
			isIgnorePrefix = s[i - 1] == Font::IGNORE_PREFIX;
		}
		if (c == '&')
		{
			auto o = tryEntityToColor(s.substr(i));
			if (o.has_value())
			{
				if (isBorderPrefix || isIgnorePrefix)
					out = out.substr(0, out.size() - 2); //remvoe even the borderprefix
				if (!isIgnorePrefix)
				{
					if (i > s.size() - 2)
						return out;
					i += 1;
					continue;
				}
			}
		}
		else if (c == '#')
		{
			auto o = tryEntityToColor(s.substr(i));
			if (o.has_value())
			{
				if (isBorderPrefix || isIgnorePrefix)
					out = out.substr(0, out.size() - 2); //remvoe even the borderprefix
				if (!isIgnorePrefix)
				{
					if (i > s.size() - 7)
						return out;
					i += 6;
					continue;
				}
			}
		}
		out += c;
	}
	return out;
}

void Font::bakeUVChars()
{
	//flips Y axis and calculates UV
	for (auto& ch : chars)
	{
		ch.u1 = (ch.u + ch.width) / scaleW;
		ch.v1 = (ch.v + ch.height) / scaleH;

		ch.u /= scaleW;
		ch.v /= scaleH;

		ch.v1 = 1 - ch.v1;
		ch.v = 1 - ch.v;
		auto foo = ch.v;
		ch.v = ch.v1;
		ch.v1 = foo;

		ch.yoffset = lineHeight - ch.yoffset - ch.height;
	}
}

bool FontParser::parse(Font& font, const std::string& filePath)
{
	font = {};

	int charCount = 0;
	auto stream = std::ifstream(ND_RESLOC(filePath));
	if (!stream.is_open())
	{
#ifdef ND_DEBUG
		ND_WARN("Invalid font path: {} / {}", filePath, ND_RESLOC(filePath));
#endif
		return false;
	}
	std::string line;
	std::vector<std::string> words;
	while (!stream.eof())
	{
		std::getline(stream, line);
		words.clear();
		if (line.empty())
			continue;;

		NDUtil::splitString(line, words);
		auto& title = words[0];

		if (title == "info")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);
				if (var._Starts_with("face="))
					font.face = arg.substr(1, arg.size() - 2);
				else if (var._Starts_with("size="))
					font.size = std::stoi(arg);
				else if (var._Starts_with("bold="))
					font.bold = (bool)std::stoi(arg);
				else if (var._Starts_with("italic="))
					font.italic = (bool)std::stoi(arg);
				else if (var._Starts_with("padding="))
				{
					std::vector<std::string> data;
					NDUtil::splitString(arg, data, ",");
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
		else if (title == "common")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);

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
				auto arg = var.substr(var.find_first_of('=') + 1);

				if (var._Starts_with("file="))
					font.texturePath = arg.substr(1, arg.size() - 2);;
			}
		}
		else if (title == "chars")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);

				if (var._Starts_with("count="))
					charCount = std::stoi(arg);
			}
		}
		else if (title == "char")
		{
			Character ch = {};
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);

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
		else if (title == "kernings")
		{
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);

				if (var._Starts_with("count="))
					font.kerning.reserve(std::stoi(arg));
			}
		}
		else if (title == "kerning")
		{
			int first = 0, second, amount;
			for (int i = 1; i < words.size(); ++i)
			{
				auto& var = words[i];
				auto arg = var.substr(var.find_first_of('=') + 1);

				if (var._Starts_with("first="))
					first = std::stoi(arg);
				else if (var._Starts_with("second="))
					second = std::stoi(arg);
				else if (var._Starts_with("amount="))
					amount = std::stoi(arg);
			}
			font.setKerning(first, second, amount);
		}
	}
#ifdef ND_DEBUG
	if(font.chars.size() != charCount)
		ND_WARN("Strange font file {}",ND_RESLOC(filePath));
#endif
	font.bakeUVChars();
	return true;
}
