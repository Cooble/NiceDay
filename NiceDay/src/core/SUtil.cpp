#include "SUtil.h"

/*bool SUtil::replaceWith(Stringo& src, const char* what, const char* with)
{
	Stringo out;
	size_t whatlen = strlen(what);
	out.reserve(src.size());
	size_t ind = 0;
	size_t lastInd = 0;
	while (true)
	{
		ind = src.find(what, ind);
		if (ind == Stringo::npos) {
			out += src.substr(lastInd);
			break;
		}
		out += src.substr(lastInd, ind- lastInd) + with;
		ind += whatlen;
		lastInd = ind;
	}
	src = out;
	return true;
}*/
namespace nd::SUtil {
const int* utf8toCodePointsArray(const char* c, int* length)
{
	//todo use something better than std::vector
	std::vector<int> out;
	char byte1 = 0;
	while ((byte1 = *c++) != 0)
	{
		if (!(byte1 & 0b10000000))
		{
			out.push_back(byte1);
		}
		if ((byte1 & 0b11100000) == 0b11000000)
		{
			//starts with 110
			char byte2 = *c++;
			int b1 = (int)byte1 & 0b00011111;
			int b2 = (int)byte2 & 0b00111111;

			out.push_back(b2 | (b1 << 6));
		}
		else if ((byte1 & 0b11110000) == 0b11100000)
		{
			//starts with 1110
			char byte2 = *c++;
			char byte3 = *c++;

			int b1 = (int)byte1 & 0b00001111;
			int b2 = (int)byte2 & 0b00111111;
			int b3 = (int)byte3 & 0b00111111;

			out.push_back(b3 | (b2 << 6) | (b1 << 6));
		}
		else if ((byte1 & 0b11111000) == 0b11110000)
		{
			//starts with 1110
			char byte2 = *c++;
			char byte3 = *c++;
			char byte4 = *c++;

			int b1 = (int)byte1 & 0b00000111;
			int b2 = (int)byte2 & 0b00111111;
			int b3 = (int)byte3 & 0b00111111;
			int b4 = (int)byte4 & 0b00111111;

			out.push_back(b4 | (b3 << 6) | (b2 << 12) | (b1 << 18));
		}
	}
	if (length)
		*length = out.size();
	if (out.empty())
		return nullptr;

	out.push_back(0);
	auto o = (int*)malloc(out.size() * sizeof(int));
	memcpy(o, out.data(), out.size() * sizeof(int));
	return o;
}

std::u32string utf8toCodePoints(const char* c)
{
	//todo use something better than std::vector
	std::u32string out;
	uint8_t byte1;
	while ((byte1 = *c++) != 0)
	{
		if (!(byte1 & 0b10000000))
		{
			out.push_back(byte1);
		}
		if ((byte1 & 0b11100000) == 0b11000000)
		{
			//starts with 110
			uint8_t byte2 = *c++;
			int b1 = (int)byte1 & 0b00011111;
			int b2 = (int)byte2 & 0b00111111;

			out.push_back(b2 | (b1 << 6));
		}
		else if ((byte1 & 0b11110000) == 0b11100000)
		{
			//starts with 1110
			uint8_t byte2 = *c++;
			uint8_t byte3 = *c++;

			int b1 = (int)byte1 & 0b00001111;
			int b2 = (int)byte2 & 0b00111111;
			int b3 = (int)byte3 & 0b00111111;

			out.push_back(b3 | (b2 << 6) | (b1 << 12));
		}
		else if ((byte1 & 0b11111000) == 0b11110000)
		{
			//starts with 11110
			uint8_t byte2 = *c++;
			uint8_t byte3 = *c++;
			uint8_t byte4 = *c++;

			int b1 = (int)byte1 & 0b00000111;
			int b2 = (int)byte2 & 0b00111111;
			int b3 = (int)byte3 & 0b00111111;
			int b4 = (int)byte4 & 0b00111111;

			out.push_back(b4 | (b3 << 6) | (b2 << 12) | (b1 << 18));
		}
	}
	return out;
}

std::string u32StringToString(std::u32string_view s)
{
	std::string out;
	for (auto c : s)
		out += (char)c;
	return out;
}
}
