#pragma once

#include "physShapes.h"
struct NBT;

class NBTSaveable //todo those methods should be inlined
{
	virtual void save(NBT& src) = 0;
	virtual void load(NBT& src) = 0;
};

#define NBT_BUILD_FUNC(typeName,listName,internalListType)\
	template <>\
	typeName& get<>(const std::string& s, const typeName& defaultVal)\
	{\
		auto it = listName.find(s);\
		if (it != listName.end())\
			return *((typeName*)&it->second);\
		listName[s] = *(internalListType*)&defaultVal;\
		return *((typeName*)&listName[s]);\
	}\
	template <>\
		typeName& get<>(const std::string& s)\
	{\
		auto it = listName.find(s); \
		if (it != listName.end())\
			return *((typeName*)&it->second); \
		return *((typeName*)&listName[s]); \
	}\
	template <>\
	bool exists<typeName>(const std::string& s)\
	{\
		auto it = listName.find(s); \
		return it != listName.end();\
	}\
	template <>\
	void set<typeName>(const std::string& s,const typeName& value)\
	{\
		listName[s] = *(internalListType*)&value; \
	}


struct NBT
{
	std::unordered_map<std::string, std::string> m_strings;
	std::unordered_map<std::string, int8_t> m_bytes;
	std::unordered_map<std::string, int32_t> m_ints;
	std::unordered_map<std::string, int64_t> m_longs;
	std::unordered_map<std::string, NBT> m_nbts;
public:
	
	NBT() = default;

	template <typename T>
	T& get(const std::string& s, const T& defaultVal)
	{
		ASSERT(false, "INVALID NBT TYPE");
	}

	template <typename T>
	T& get(const std::string& s)
	{
		ASSERT(false, "INVALID NBT TYPE");
	}

	template <typename T>
	bool exists(const std::string& s)
	{
		ASSERT(false, "INVALID NBT TYPE");
	}

	template <typename T>
	void set(const std::string& s, const T& value)
	{
		ASSERT(false, "INVALID NBT TYPE");
	}

private:
	struct NBTHeader
	{
		size_t string_count;
		size_t bytes_count;
		size_t ints_count;
		size_t long_count;
		size_t nbt_count;
	};

public:

	void save(std::fstream& stream)
	{
		NBTHeader data = {};
		data.string_count = m_strings.size();
		data.bytes_count = m_bytes.size();
		data.ints_count = m_ints.size();
		data.long_count = m_longs.size();
		data.nbt_count = m_nbts.size();
		stream.write((const char*)&data, sizeof(NBTHeader));

		for (auto& tt : m_strings)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			stream.write(tt.second.data(), tt.second.size()+1);
		}
		for (auto tt : m_bytes)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			stream.write((char*)&tt.second, 1);
		}
		for (auto tt : m_ints)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			stream.write((char*)&tt.second, 4);
		}
		for (auto tt : m_longs)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			stream.write((char*)&tt.second, 8);
		}
		for (auto& tt : m_nbts)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			tt.second.save(stream);
		}
	}

	inline void readString(std::fstream& stream, char* buff, size_t& size)
	{
		size = 0;
		char c = 0;
		stream.read(&c, 1);
		while (c != 0)
		{
			buff[size++] = c;
			stream.read(&c, 1);
			ASSERT(size < 2048,"string Buffer overflow");
			
		}
		
		//buff[size++] = 0; //null character
	}

	void load(std::fstream& stream, char* buff = nullptr)
	{
		bool allocated = false;
		if (buff == nullptr)
		{
			buff = new char[2048];
			allocated = true;
		}
		size_t size;
		NBTHeader data = {};
		stream.read((char*)&data, sizeof(NBTHeader));

		for (int i = 0; i < data.string_count; ++i)
		{
			readString(stream, buff, size);
			auto key = std::string(buff, buff + size);
			readString(stream, buff, size);
			auto val = std::string(buff, buff + size);
			m_strings[key] = val;
		}


		for (int i = 0; i < data.bytes_count; ++i)
		{
			readString(stream, buff, size);
			auto key = std::string(buff, buff + size);

			m_bytes[key] = (char)stream.get();
		}
		char bufff[8];
		for (int i = 0; i < data.ints_count; ++i)
		{
			readString(stream, buff, size);
			auto key = std::string(buff, buff + size);
			stream.read(bufff, 4);
			m_ints[key] = *(int*)&bufff;
		}
		for (int i = 0; i < data.long_count; ++i)
		{
			readString(stream, buff, size);
			auto key = std::string(buff, buff + size);
			stream.read(bufff, 8);
			m_longs[key] = *(int64_t*)&bufff;
		}
		for (int i = 0; i < data.nbt_count; ++i)
		{
			readString(stream, buff, size);
			auto key = std::string(buff, buff + size);
			get<NBT>(key).load(stream, buff);
		}
		if (allocated)
			delete[] buff;
	}

	//nbt =======================================

	NBT_BUILD_FUNC(NBT, m_nbts,NBT)

	//string=====================================

	NBT_BUILD_FUNC(std::string, m_strings,std::string)

	//1byte=====================================

	NBT_BUILD_FUNC(uint8_t, m_bytes, int8_t)
	NBT_BUILD_FUNC(int8_t, m_bytes, int8_t)

	//4bytes======================================

	NBT_BUILD_FUNC(uint32_t, m_ints, int32_t)
	NBT_BUILD_FUNC(float, m_ints, int32_t)
	NBT_BUILD_FUNC(int32_t, m_ints, int32_t)


	//8bytes======================================

	NBT_BUILD_FUNC(uint64_t, m_longs, int64_t)
	NBT_BUILD_FUNC(int64_t, m_longs, int64_t)
	NBT_BUILD_FUNC(Phys::Vect, m_longs, int64_t)
	NBT_BUILD_FUNC(glm::vec2, m_longs, int64_t)

	~NBT() = default;
};
