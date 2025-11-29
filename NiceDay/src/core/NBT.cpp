#include "ndpch.h"
#include "NBT.h"
#include <nlohmann/json.hpp>

namespace nd {

Stringo NBT::dump(int depth) const
{
	Stringo tabs;
	tabs.reserve(depth);
	for (int i = 0; i < depth; ++i)
		tabs += "\t";
	Stringo s;
	if (isString())
		s = "\"" + *val_string + "\"";
	else if (isFloat())
		s = std::to_string(val_float);
	else if (isInt())
		s = std::to_string(val_int);
	else if (isUInt())
		s = std::to_string(val_uint);
	else if (isBool())
		s = std::to_string((bool)val_int);
	else if (isMap())
	{
		s = "{\n";
		for (auto& map : maps())
		{
			s += tabs + "\t\"" + map.first + "\": " + map.second.dump(depth + 1) + ",\n";
		}
		s = s.substr(0, s.size() - 2); //remove last comma and backslash
		s += "\n" + tabs + "}";
	}
	else if (isArray())
	{
		s = "[\n";
		for (auto& map : arrays())
		{
			s += tabs + "\t" + map.dump(depth + 1) + ",\n";
		}
		s = s.substr(0, s.size() - 2); //remove last comma and backslash
		s += "\n" + tabs + "]";
	}
	else
	{
		s = "NULL";
	}
	return s;
}

json NBT::toJson() const
{
	if (isString())
		return json(*val_string);
	if (isFloat())
		return json(val_float);
	if (isInt())
		return json(val_int);
	if (isUInt())
		return json("#" + std::to_string(val_uint));
	if (isBool())
		return json((bool)val_int);
	if (isMap())
	{
		json out;
		for (auto& map : maps())
			out[map.first] = map.second.toJson();
		return out;
	}
	if (isArray())
	{
		json out;
		for (int i = 0; i < size(); ++i)
			out[i] = (*val_array)[i].toJson();
		return out;
	}
	return json();
}

NBT NBT::fromJson(const json& j)
{
	if (j.is_string())
		return NBT(Stringo(j));
	if (j.is_number_float())
		return NBT(double(j));
	if (j.is_number_integer())
		return NBT(int64_t(j));
	if (j.is_number_unsigned())
		return NBT(uint64_t(j));
	if (j.is_boolean())
		return NBT(bool(j));
	if (j.is_object())
	{
		NBT out;
		for (auto& map : j.items())
			out[map.key()] = NBT::fromJson(map.value());
		return out;
	}
	if (j.is_array())
	{
		NBT out;
		out.resize(j.size());
		for (int i = 0; i < j.size(); ++i)
			out[i] = NBT::fromJson(j[i]);
		return out;
	}
	return NBT();
}

void NBT::saveToFile(const Stringo& filePath, const NBT& nbt)
{
	std::ofstream o(ND_RESLOC(filePath));
	o << std::setw(4) << nbt.toJson() << std::endl;
	o.close();
}


constexpr int BIG_JSON_BUFF_SIZE = 500000;
static char BIG_JSON_BUFFER[BIG_JSON_BUFF_SIZE];
//removes comments: (replaces with ' ')
//	1. one liner starting with "//"
//	2. block comment bounded by "/*" and "*/"
static void removeComments(int length)
{
	int startIndex = -1;
	bool bigLiner = false;
	char lastChar = ' ';
	for (int i = 0; i < length; ++i)
	{
		auto val = BIG_JSON_BUFFER[i];
		if (val == '\0')
		{
			if (startIndex != -1)
			{
				memset(&BIG_JSON_BUFFER[startIndex], ' ', i - startIndex);
			}
			return;
		}
		if (startIndex == -1)
		{
			if (val == '/')
			{
				if (lastChar == '/')
				{
					startIndex = i - 1;
					bigLiner = false;
				}
			}
			else if (val == '*')
			{
				if (lastChar == '/')
				{
					startIndex = i - 1;
					bigLiner = true;
				}
			}
		}
		else if (val == '\n' && !bigLiner)
		{
			memset(&BIG_JSON_BUFFER[startIndex], ' ', i - startIndex);
			startIndex = -1;
		}
		else if (val == '/' && lastChar == '*' && bigLiner)
		{
			memset(&BIG_JSON_BUFFER[startIndex], ' ', i - startIndex + 1);
			startIndex = -1;
		}

		lastChar = val;
	}
}

bool NBT::loadFromFile(const Stringo& filePath, NBT& nbt)
{
	std::ifstream o(ND_RESLOC(filePath));
	if (!o.is_open())
		return false;

	o.read(BIG_JSON_BUFFER, BIG_JSON_BUFF_SIZE);
	if (o.eof())
	{
		// got the whole file...
		removeComments(o.gcount());
		int red = o.gcount();
		BIG_JSON_BUFFER[red] = '\0';
		BIG_JSON_BUFFER[red + 1] = '\0';
		json j;
		try
		{
			j = json::parse(BIG_JSON_BUFFER);
		}
		catch (...)
		{
			ND_ERROR("JSON Error when parsing: {}", filePath);
			o.close();
			return false;
		}
		o.close();
		nbt = NBT::fromJson(j);
		return true;
	}
	ND_WARN("Cannot load file {}, its too big", filePath);
	return false;
}

bool NBT::saveAsCSV(const Stringo& filePath, const NBT& nbt, char separator, const char* idxName)
{
	if (!nbt.isContainer())
	{
		ND_ERROR("NBT is not an array nor map");
		return false;
	}

	// in case of array its simply 0,1,2,3...
	// in case of map we need to extract keys (which must be ints) and sort them
	std::vector<int> indexes;
	if (nbt.isMap())
	{
		// need to convert
		for (auto& [key,val] : nbt.maps())
		{
			try
			{
				int index = std::stoi(key);
				indexes.push_back(index);
			}
			catch (...)
			{
				ND_ERROR("NBT map key is not an integer: {}", key);
				return false;
			}
		}
	}
	std::sort(indexes.begin(), indexes.end());


	// first headers
	auto& firstMap = nbt.isArray() ? nbt.arrays()[0].maps() : nbt[std::to_string(indexes[0])].maps();
	std::ofstream o(ND_RESLOC(filePath));

	if (idxName && strlen(idxName))
		o << idxName << separator;

	for (auto& map : firstMap)
		o << "\"" << map.first << "\"" << separator;
	o.seekp(-1, std::ios_base::cur); //remove last comma
	o << "\n";

	for (int i = 0;i < nbt.size();i++)
	{
		auto& item = nbt.isArray() ? nbt.arrays()[i] : nbt[std::to_string(indexes[i])];

		// always write index as first column
		o << (nbt.isArray() ? i : indexes[i]) << separator;

		for (auto& [key,_] : firstMap)
		{
			if (item.exists(key))
			{
				auto& val = item.access_map_const(key);
				if (val.isString())
					o << "\"" << val.string() << "\"" << separator;
				else if (val.isFloat())
					o << val.val_float << separator;
				else if (val.isInt())
					o << val.val_int << separator;
				else if (val.isUInt())
					o << val.val_uint << separator;
				else if (val.isBool())
					o << (bool)val.val_int << separator;
				else
					o << "\"\"" <<separator;
			}
			else
			{
				o << "\"\"" << separator;
			}
		}
		o.seekp(-1, std::ios_base::cur); //remove last comma
		o << "\n";
	}
	o << "\n";
	return true;
}

void NBT::write(const IBinaryStream::RWStream& write) const
{
	BinarySerializer::write(*this, write.m_write);
}

void NBT::read(const IBinaryStream::RWStream& read)
{
	BinarySerializer::read(*this, read.m_read);
}

bool operator==(const NBT& a, const NBT& b)
{
	if (a.isString() && b.isString())
		return a.string() == b.string();
	if (a.isFloat() && b.isFloat())
		return double(a) == double(b);
	if (a.isInt() && b.isInt())
		return int64_t(a) == int64_t(b);
	if (a.isUInt() && b.isUInt())
		return uint64_t(a) == uint64_t(b);
	if (a.isBool() && b.isBool())
		return bool(a) == bool(b);
	if (a.isNull() && b.isNull())
		return true;

	if (a.size() != b.size())
		return false;

	if (a.isMap() && b.isMap())
	{
		for (auto& map : a.maps())
			if (!b.exists(map.first) || !operator==(b.access_map_const(map.first), map.second))
				return false;
		return true;
	}
	if (a.isArray() && b.isArray())
	{
		for (int i = 0; i < a.size(); ++i)
			if (!operator==(b[i], a[i]))
				return false;
		return true;
	}
	return false;
}

constexpr uint8_t BB_ARRAY_OPEN = 0xf0;
constexpr uint8_t BB_ARRAY_CLOSE = 0xf1;
constexpr uint8_t BB_MAP_OPEN = 0xf2;
constexpr uint8_t BB_MAP_CLOSE = 0xf3;

constexpr uint8_t BB_INT = 0xf4;
constexpr uint8_t BB_UINT = 0xf5;
constexpr uint8_t BB_FLOAT = 0xf6;
constexpr uint8_t BB_BOOL = 0xf7;
constexpr uint8_t BB_STRING = 0xf8;
constexpr uint8_t BB_NULL = 0xf9;

void BinarySerializer::write(const NBT& a, const IBinaryStream::WriteFunc& write)
{
	char bu[16];
	if (a.isString())
	{
		bu[0] = BB_STRING;
		*(uint32_t*)&bu[1] = a.string().size();
		write(bu, 5);
		write(a.string().c_str(), a.string().size());
	}
	else if (a.isFloat())
	{
		bu[0] = BB_FLOAT;
		*(double*)&bu[1] = (double)a;
		write(bu, 9);
	}
	else if (a.isInt())
	{
		bu[0] = BB_INT;
		*(int64_t*)&bu[1] = (int64_t)a;
		write(bu, 9);
	}
	else if (a.isUInt())
	{
		bu[0] = BB_UINT;
		*(uint64_t*)&bu[1] = (uint64_t)a;
		write(bu, 9);
	}
	else if (a.isBool())
	{
		bu[0] = BB_BOOL;
		*&bu[1] = (bool)a;
		write(bu, 2);
	}
	if (a.isMap())
	{
		bu[0] = BB_MAP_OPEN;
		write(bu, 1);
		for (auto& map : a.maps())
		{
			ASSERT(map.first.size() < 256, "The key is too long (over 255): {}", map.first.size());

			*(uint8_t*)&bu[0] = BB_STRING;
			*(uint8_t*)&bu[1] = (uint8_t)map.first.size();
			write(bu, 2);
			write(map.first.c_str(), map.first.size());
			BinarySerializer::write(map.second, write);
		}
		bu[0] = BB_MAP_CLOSE;
		write(bu, 1);
	}
	if (a.isArray())
	{
		bu[0] = BB_ARRAY_OPEN;
		write(bu, 1);
		for (auto& map : a.arrays())
		{
			BinarySerializer::write(map, write);
		}
		bu[0] = BB_ARRAY_CLOSE;
		write(bu, 1);
	}
	else if (a.isNull())
	{
		bu[0] = BB_NULL;
		write(bu, 1);
	}
}

constexpr uint32_t buffSize = 4096;
char buffer[buffSize];

bool BinarySerializer::read(NBT& n, const IBinaryStream::ReadFunc& read)
{
	char buff[16];
	uint8_t type;
	read((char*)&type, 1);
	switch (type)
	{
	case BB_STRING:
		{
			{
				Stringo s;
				uint32_t size;
				read((char*)&size, 4);
				uint32_t tempSize = size;
				while (tempSize != 0)
				{
					uint32_t toread = std::min(buffSize, tempSize);
					tempSize -= toread;
					read((char*)&buffer, toread);
					s += Stringo(buffer, toread);
				}
				n = s;
			}
		}
		break;
	case BB_INT:
		{
			int64_t val;
			read((char*)&val, 8);
			n = val;
		}
		break;
	case BB_UINT:
		{
			uint64_t val;
			read((char*)&val, 8);
			n = val;
		}
		break;
	case BB_FLOAT:
		{
			double val;
			read((char*)&val, 8);
			n = val;
		}
		break;
	case BB_BOOL:
		{
			char val;
			read(&val, 1);
			n = (bool)val;
		}
		break;
	case BB_MAP_OPEN:
		{
			while (true)
			{
				read((char*)&type, 1);
				if (type == BB_MAP_CLOSE)
					break;
				//read size of key string
				uint8_t size;
				read((char*)&size, 1);
				read(buffer, size);
				auto s = Stringo(buffer, size);
				NBT val;
				BinarySerializer::read(val, read);
				n[s] = std::move(val);
			}
		}
		break;
	case BB_ARRAY_OPEN:
		{
			int index = 0;
			while (true)
			{
				NBT val;
				if (BinarySerializer::read(val, read))
					break;
				n[index++] = std::move(val);
			}
		}
		break;
	case BB_ARRAY_CLOSE:
	case BB_MAP_CLOSE:
		return true;
	}
	return false;
}
}
