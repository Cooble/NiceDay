#pragma once
#include <nlohmann/json_fwd.hpp>
#include "IBinaryStream.h"
class NBT;
typedef std::unordered_map<std::string, NBT> NBTMap;
typedef std::vector<NBT> NBTVector;
static std::string NBT_INVALID_STRING = "invalidstring";

class NBT
{
	enum NBTType : uint8_t
	{
		T_NUMBER_FLOAT = 1,
		T_NUMBER_INT = 2,
		T_NUMBER_UINT = 4,
		T_NUMBER = T_NUMBER_FLOAT | T_NUMBER_INT | T_NUMBER_UINT,
		T_STRING = 8,
		T_BOOL = 16,
		T_MAP = 32,
		T_ARRAY = 64,
		T_NULL = 0
	} type = T_NULL;

	union
	{
		double val_float;
		int64_t val_int;
		uint64_t val_uint;
		NBTVector* val_array;
		NBTMap* val_map;
		std::string* val_string;
	};


private:
	static NBT& nullVal() { static NBT s; return s; }
	constexpr bool checkForArray()
	{
		if (isNull())
		{
			type = T_ARRAY;
			this->val_array = new NBTVector();
		}
		else if (!isArray())
		{
			ND_WARN("is not an array");
			return false;
		}
		return true;
	}

	constexpr bool checkForMap()
	{
		if (isNull())
		{
			type = T_MAP;
			this->val_map = new NBTMap();
		}
		else if (!isMap())
		{
			ND_WARN("is not an map");
			return false;
		}
		return true;
	}
	constexpr void destruct()
	{
		bool des = isArray() || isMap() || isString();
		if (isArray())
			delete val_array;
		else if (isMap())
			delete val_map;
		else if (isString())
			delete val_string;
		if (des) {
			val_array = nullptr;
			type = T_NULL;
		}
	}
public:

	constexpr bool isMap() const { return type == T_MAP; }
	constexpr bool isArray() const { return type == T_ARRAY; }
	constexpr bool isNumber() const { return (type & T_NUMBER) != 0; }
	constexpr bool isFloat() const { return type == T_NUMBER_FLOAT; }
	constexpr bool isInt() const { return type == T_NUMBER_INT; }
	constexpr bool isUInt() const { return type == T_NUMBER_UINT; }
	constexpr bool isString() const { return type == T_STRING; }
	constexpr bool isBool() const { return type == T_BOOL; }
	constexpr bool isNull() const { return type == T_NULL; }
	constexpr size_t size() const { return isMap() ? val_map->size() : (isArray() ? val_array->size() : false); }
	inline NBT() = default;
	~NBT(){
		destruct();
	}

	inline NBT(int64_t i)
	{
		val_int = i;
		type = T_NUMBER_INT;
	}
	
	inline NBT(const glm::vec2& i)
	{
		val_uint = *(uint64_t*)&i;
		type = T_NUMBER_UINT;
	}
	inline NBT(uint64_t i)
	{
		val_uint = i;
		type = T_NUMBER_UINT;
	}
	inline NBT(double i)
	{
		val_float = i;
		type = T_NUMBER_FLOAT;
	}
	inline NBT(int i)
	{
		val_int = i;
		type = T_NUMBER_INT;
	}
	inline NBT(float i)
	{
		val_float = (double)i;
		type = T_NUMBER_FLOAT;
	}
	inline NBT(bool i)
	{
		val_int = i;
		type = T_BOOL;
	}

	int getDigit(char c)
	{
		return c - '0';
	}
	uint64_t toUint64(const std::string& s)
	{
		uint64_t out = 0;
		uint64_t tens = 1;
		
		for (int i = s.size()-1; i>=0; --i)
		{
			out += tens * getDigit(s[i]);
			tens *= 10;
		}
		return out;
	}
	bool isNumeric(const std::string& string)
	{
		std::size_t pos;
		long double value = 0.0;
		try
		{
			value = std::stold(string, &pos);
		}
		catch (...)
		{
			return false;
		}
		return pos == string.size() && !std::isnan(value);
	}
	inline NBT(const std::string& i)
	{
		if(i._Starts_with("#")&&isNumeric(i.substr(1)))
		{
			val_uint = toUint64(i.substr(1));
			type = T_NUMBER_UINT;
			return;
		}
		val_string = new std::string(i);
		type = T_STRING;
	}
	inline NBT(const char* i):NBT(std::string(i)){}

	NBT& operator=(bool i)
	{
		destruct();
		val_int = i;
		type = T_BOOL;
		return *this;
	}
	NBT& operator=(char i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(uint8_t i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(int16_t i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(uint16_t i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(int i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(uint32_t i)
	{
		destruct();
		val_int = (int64_t)i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(int64_t i)
	{
		destruct();
		val_int = i;
		type = T_NUMBER_INT;
		return *this;
	}
	NBT& operator=(uint64_t i)
	{
		destruct();
		val_uint = i;
		type = T_NUMBER_UINT;
		return *this;
	}
	NBT& operator=(const glm::vec2& i)
	{
		destruct();
		val_uint = *(uint64_t*)&i;
		type = T_NUMBER_UINT;
		return *this;
	}
	NBT& operator=(double i)
	{
		destruct();
		val_float = i;
		type = T_NUMBER_FLOAT;
		return *this;
	}
	NBT& operator=(float i)
	{
		destruct();
		val_float = (double)i;
		type = T_NUMBER_FLOAT;
		return *this;
	}
	NBT& operator=(const std::string& i)
	{
		if (type == T_STRING)
			*val_string = i;
		else {
			destruct();
			val_string = new std::string(i);
		}
		type = T_STRING;
		return *this;
	}
	inline int invalidCast() const { ASSERT(false, "invalid nbt type cast"); return 0; }
	inline glm::vec2 invalidCastGLM() const { ASSERT(false, "invalid nbt type cast"); return glm::vec2(0,0); }
	NBT& operator=(const char* i)
	{
		return this->operator=(std::string(i));
	}
	operator bool() const { return isBool() ? (bool)val_int : false; }
	operator char() const { return isInt() ? (char)val_int : 0; }
	operator uint8_t() const
	{
		return operator uint32_t();
	}
	operator uint16_t() const
	{
		return operator uint32_t();
	}
	operator int16_t() const
	{
		return operator int();
	}
	operator int() const
	{
		return isInt() ? val_int : (isUInt() ? val_uint : invalidCast());
	}
	operator uint32_t() const
	{
		return isInt() ? val_int : (isUInt() ? val_uint : invalidCast());
	}
	operator int64_t() const
	{
		return isInt() ? val_int : (isUInt() ? val_uint : invalidCast());
	}
	operator uint64_t() const
	{
		return isUInt() ? val_uint : (isInt()?val_int:invalidCast());
	}
	operator glm::vec2() const
	{
		return isUInt() ? (*(glm::vec2*)&val_uint): invalidCastGLM();
	}

	operator float() const { return isFloat()  ? val_float : (isInt()?val_int:invalidCast()); }
	operator double() const { return isFloat() ? val_float : (isInt() ? val_int : invalidCast()); }

	std::string& string() { return isString() ? *val_string : NBT_INVALID_STRING; }
	const std::string& string() const { return isString() ? *val_string : NBT_INVALID_STRING; }


	//move
	inline NBT(NBT&& o) noexcept : type(o.type)
	{
		val_int = o.val_int;
		o.val_int = 0;
		o.type = T_NULL;
	}

	//copy
	inline NBT(const NBT& o) : type(o.type)
	{
		val_int = o.val_int;
		type = o.type;
		if (o.type == T_MAP)
			val_map = new NBTMap(*o.val_map);
		else if (o.type == T_ARRAY)
			val_array = new NBTVector(*o.val_array);
		else if (o.type == T_STRING)
			val_string = new std::string(*o.val_string);
	}

	//copy
	NBT& operator=(const NBT& o)
	{
		val_int = o.val_int;
		type = o.type;
		if (o.type == T_MAP)
			val_map = new NBTMap(*o.val_map);
		else if (o.type == T_ARRAY)
			val_array = new NBTVector(*o.val_array);
		else if (o.type == T_STRING)
			val_string = new std::string(*o.val_string);
		return *this;
	}

	//move
	NBT& operator=(NBT&& o) noexcept
	{
		destruct();//todo not quite sure
		val_int = o.val_int;
		type = o.type;
		o.val_int = 0;
		o.type = T_NULL;

		return *this;
	}

	void push_back(NBT&& nbt)
	{
		if (!checkForArray())
			return;
		val_array->push_back(std::move(nbt));
	}
	void push_back(const NBT& nbt)
	{
		if (!checkForArray())
			return;
		val_array->push_back(nbt);
	}

	template <typename Arg>
	void emplace_back(Arg&& arg)
	{
		if (!checkForArray())
			return;
		val_array->emplace_back(std::forward<Arg>(arg));
	}
	template <typename Arg>
	void emplace_map(const char* name, Arg&& arg)
	{
		if (!checkForMap())
			return;
		val_map->try_emplace(name, std::forward<Arg>(arg));
	}
	template <typename Arg>
	void emplace_map(const std::string& name, Arg&& arg)
	{
		if (!checkForMap())
			return;
		val_map->try_emplace(name, std::forward<Arg>(arg));
	}

	//map access
	inline NBT& access_map(const std::string& name)
	{
		if (!checkForMap())
		{
			ASSERT(false, "");
			return nullVal();
		}
		auto& it = val_map->find(name);
		if (it == val_map->end())
			return val_map->insert_or_assign(name, NBT()).first->second;
		return it->second;
	}
	inline const NBT& access_map_const(const std::string& name) const
	{
		if (!isMap())
		{
			ASSERT(false, "");
			return nullVal();
		}
		return val_map->operator[](std::forward<const std::string>(name));
	}

	NBT& operator[](const std::string& name)
	{
		return access_map(name);
	}
	NBT& operator[](const char* name)
	{
		return access_map(std::string(name));
	}
	const NBT& operator[](const std::string&& name) const
	{
		return access_map_const(name);
	}
	const NBT& operator[](const char* name) const
	{
		return access_map_const(std::string(name));
	}


	void reserve(int size) { if (checkForArray())val_array->reserve(size); }
	void resize(size_t size){ if (checkForArray())val_array->resize(size); }
	bool exists(const std::string& key) const
	{
		return isMap() && (val_map->find(key) != val_map->end());
	}
	
	//array access
	NBT& operator[](int index)
	{
		if (!checkForArray())
		{
			ASSERT(false, "");
			return nullVal();
		}
		while (val_array->size() <= index)
			val_array->push_back(NBT());
		return val_array->operator[](index);
	}

	const NBT& operator[](int index) const
	{
		if (!isArray())
		{
			ASSERT(false, "");
			return nullVal();
		}
		return val_array->operator[](index);
	}

	auto& arrays()
	{
		ASSERT(checkForArray(), "");
		return *val_array;
	}
	auto& maps()
	{
		ASSERT(checkForMap(), "");
		return *val_map;
	}

	auto& arrays() const
	{
		ASSERT(isArray(), "");
		return *val_array;
	}
	auto& maps() const
	{
		ASSERT(isMap(), "");
		return *val_map;
	}

	std::string dump() const
	{
		return dump(0);
	}

	std::string dump(int depth) const;

	json toJson() const;
	static NBT fromJson(const json& j);
	
	static void saveToFile(const std::string& filePath, const NBT& nbt);
	static bool loadFromFile(const std::string& filePath, NBT& nbt);

	template <typename Arg>
	void save(const std::string& name,const Arg& val)
	{
		if (!checkForMap())
			return;
		operator[](name) = val;
	}

	template <typename Arg>
	bool load(const std::string& key, Arg& val, Arg defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = Arg(it->second);
		else val = defaultVal;
		return found;
	}
	template <>
	bool load(const std::string& key, std::string& val, std::string defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = it->second.string();
		else val = defaultVal;
		return found;
	}
	/*
	bool load(const std::string& key, std::string& val, const char* defaultVal) const
	{
		return load(key, val, std::string(defaultVal));
	}*/

	template <typename Arg>
	bool load(const std::string& key, Arg& val) const
	{
		if (!isMap())
			return false;
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = Arg(it->second);
		return found;
	}
	template <>
	bool load(const std::string& key, std::string& val) const
	{
		if (!isMap())
			return false;
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found&&it->second.isString())
			val = it->second.string();
		return found;
	}

	void write(const IBinaryStream::RWStream& write) const;
	void read(const IBinaryStream::RWStream& read);
};

bool operator==(const NBT& a, const NBT& b);
inline bool operator!=(const NBT& a, const NBT& b) { return !operator==(a, b); }

namespace BinarySerializer
{
	//writes to a binary stream
	void write(const NBT& n, const IBinaryStream::WriteFunc& write);
	//reads from binary stream
	//ignore return value (it's for inner purposes)
	bool read(NBT& n, const IBinaryStream::ReadFunc& write);
}