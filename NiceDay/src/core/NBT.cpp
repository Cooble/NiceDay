#include "ndpch.h"
#include "NBT.h"
#include <nlohmann/json.hpp>

std::string NBT::dump(int depth) const
{
	std::string tabs;
	tabs.reserve(depth);
	for (int i = 0; i < depth; ++i)
		tabs += "\t";
	std::string s;
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
		return json("#"+std::to_string(val_uint));
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
		return NBT(std::string(j));
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
	if(j.is_array())
	{
		NBT out;
		out.resize(j.size());
		for (int i = 0; i < j.size(); ++i)
			out[i] = NBT::fromJson(j[i]);
		return out;
	}
	return NBT();
}

void NBT::saveToFile(const std::string& filePath, const NBT& nbt)
{
	std::ofstream o(ND_RESLOC(filePath));
	o << std::setw(4) << nbt.toJson() << std::endl;
	o.close();
}

bool NBT::loadFromFile(const std::string& filePath, NBT& nbt)
{
	std::ifstream o(ND_RESLOC(filePath));
	if (!o.is_open())
		return false;
	json j;
	o >> j;
	o.close();
	nbt = NBT::fromJson(j);
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
	if (a.isInt()&&b.isInt())
		return int64_t(a) == int64_t(b);
	if (a.isUInt()&&b.isUInt())
		return uint64_t(a) == uint64_t(b);
	if (a.isBool()&&b.isBool())
		return bool(a) == bool(b);
	if (a.isNull() && b.isNull())
		return true;
	
	if (a.size() != b.size())
		return false;
	
	if (a.isMap()&&b.isMap())
	{
		for (auto& map : a.maps())
			if (!b.exists(map.first) || !operator==(b.access_map_const(map.first), map.second))
				return false;
		return true;
	}
	if (a.isArray()&&b.isArray())
	{
		for (int i = 0; i < a.size(); ++i)
			if (!operator==(b[i],a[i]))
				return false;
		return true;
	}
	return false;
}

constexpr uint8_t BB_ARRAY_OPEN		=0xf0;
constexpr uint8_t BB_ARRAY_CLOSE	=0xf1;
constexpr uint8_t BB_MAP_OPEN		=0xf2;
constexpr uint8_t BB_MAP_CLOSE		=0xf3;

constexpr uint8_t BB_INT			=0xf4;
constexpr uint8_t BB_UINT			=0xf5;
constexpr uint8_t BB_FLOAT			=0xf6;
constexpr uint8_t BB_BOOL			=0xf7;
constexpr uint8_t BB_STRING			=0xf8;
constexpr uint8_t BB_NULL			=0xf9;

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
		*(char*)&bu[1] = (bool)a;
		write(bu, 2);
	}
	if (a.isMap())
	{
		bu[0] = BB_MAP_OPEN;
		write(bu, 1);
		for (auto& map : a.maps())
		{
			ASSERT(map.first.size() < 256,"The key is too long (over 255)");
			
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

constexpr uint32_t buffSize=4096;
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
			std::string s;
			uint32_t size;
			read((char*)&size, 4);
			uint32_t tempSize=size;
			while(tempSize!=0)
			{
				uint32_t toread = std::min(buffSize, tempSize);
				tempSize -= toread;
				read((char*)&buffer, toread);
				s += std::string(buffer, toread);
			}
			n = s;
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
		while(true)
		{
			read((char*)&type, 1);
			if (type == BB_MAP_CLOSE)
				break;
			//read size of key string
			uint8_t size;
			read((char*)&size, 1);
			read(buffer,size);
			auto s = std::string(buffer, size);
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
