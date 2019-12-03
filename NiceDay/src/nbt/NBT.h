#pragma once
#include "core/physShapes.h"
#include "ndpch.h"
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
		auto ss = s+m_prefix;\
		auto it = listName.find(ss);\
		if (it != listName.end())\
			return *((typeName*)&it->second);\
		listName[ss] = *(internalListType*)&defaultVal;\
		return *((typeName*)&listName[ss]);\
	}\
	template <>\
		typeName& get<>(const std::string& s)\
	{\
		auto ss = s+m_prefix;\
		auto it = listName.find(ss); \
		if (it != listName.end())\
			return *((typeName*)&it->second); \
		return *((typeName*)&listName[ss]); \
	}\
	template <>\
	bool exists<typeName>(const std::string& s)\
	{\
		auto ss = s+m_prefix;\
		auto it = listName.find(ss); \
		return it != listName.end();\
	}\
	template <>\
	void set<typeName>(const std::string& s,const typeName& value)\
	{\
		auto ss = s+m_prefix;\
		listName[ss] = *(internalListType*)&value; \
	}

class IStream
{
public:
	virtual ~IStream() = default;
	virtual void write(const char* b, uint32_t length)=0;
	virtual bool read(char* b, uint32_t length)=0;

	template<typename T>
	void write(const T& t)
	{
		write((const char*)&t, sizeof(T));
	}
	template<typename T>
	void read(T& t)
	{
		read((char*)&t, sizeof(T));
	}
};
class BasicIStream:public std::fstream,public IStream
{
public:
	virtual ~BasicIStream() = default;
	
	virtual void write(const char* b, uint32_t length) override
	{
		std::fstream::write(b, length);
	}
	virtual bool read(char* b, uint32_t length) override
	{
		std::fstream::read(b, length);
	}

};

struct NBT
{
	std::unordered_map<std::string, std::string> m_strings;
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
	template <>
	void set(const std::string& s, const std::string_view& value)
	{
		set(s, std::string(value));
	}

private:
	struct NBTHeader
	{
		size_t string_count;
		size_t ints_count;
		size_t long_count;
		size_t nbt_count;
	};
	std::string m_prefix="";

public:

	inline void setPrefix(std::string prefix) { m_prefix = prefix+":"; }
	inline void resetPrefix() { m_prefix = ""; }
	void clear()
	{
		m_strings.clear();
		m_ints.clear();
		m_longs.clear();
		m_nbts.clear();
		resetPrefix();
	}

	void serialize(IStream* strea)
	{
		auto& stream = *strea;

		NBTHeader data;
		data.string_count = m_strings.size();
		data.ints_count = m_ints.size();
		data.long_count = m_longs.size();
		data.nbt_count = m_nbts.size();
		stream.write((const char*)&data, sizeof(NBTHeader));

		for (auto& tt : m_strings)
		{
			stream.write(tt.first.c_str(), tt.first.size()+1);
			stream.write(tt.second.c_str(), tt.second.size()+1);
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
			tt.second.serialize(&stream);
		}
	}

	inline void readString(IStream& stream, char* buff, size_t& size)
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
private:
	void deserializeInner(IStream* strea, char* buff)
	{
		auto& stream = *strea;
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
			get<NBT>(key).deserializeInner(&stream, buff);
		}
		if (allocated)
			delete[] buff;
	}
public:
	void deserialize(IStream* strea)
	{
		deserializeInner(strea, nullptr);
	}

	//nbt =======================================

	NBT_BUILD_FUNC(NBT, m_nbts,NBT)

	//string=====================================

	NBT_BUILD_FUNC(std::string, m_strings,std::string)

	//1byte=====================================

	NBT_BUILD_FUNC(uint8_t, m_ints, int32_t)
	NBT_BUILD_FUNC(int8_t, m_ints, int32_t)
	
	//2byte=====================================

	NBT_BUILD_FUNC(uint16_t, m_ints, int32_t)
	NBT_BUILD_FUNC(int16_t, m_ints, int32_t)


	//4bytes======================================

	NBT_BUILD_FUNC(uint32_t, m_ints, int32_t)
	NBT_BUILD_FUNC(float, m_ints, int32_t)
	NBT_BUILD_FUNC(int32_t, m_ints, int32_t)
	NBT_BUILD_FUNC(bool, m_ints, int32_t)

	//8bytes======================================

	NBT_BUILD_FUNC(uint64_t, m_longs, int64_t)
	NBT_BUILD_FUNC(int64_t, m_longs, int64_t)
	NBT_BUILD_FUNC(Phys::Vect, m_longs, int64_t)
	NBT_BUILD_FUNC(glm::vec2, m_longs, int64_t)

	~NBT() = default;
};
