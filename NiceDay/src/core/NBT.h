#pragma once
#include "core/json_fwd.h"
#include "IBinaryStream.h"

typedef std::string Stringo;
class NBT;
typedef std::unordered_map<Stringo, NBT> NBTMap;
typedef std::vector<NBT> NBTVector;
static Stringo NBT_INVALID_STRING = "invalidstring";

class NBT
{
public:
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
private:
	union
	{
		double val_float;
		int64_t val_int;
		uint64_t val_uint;
		NBTVector* val_array;
		NBTMap* val_map;
		Stringo* val_string;
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
	constexpr void destruct() noexcept
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
	template<int vecSize>
	void buildVec(const glm::vec<vecSize, float>& v)
	{
		ASSERT(type == T_NULL || type == T_ARRAY, "");
		checkForArray();
		for (int i = 0; i < vecSize; ++i)
			access_array(i) = glm::value_ptr(v)[i];
	}
public:
	const char* c_str() const { return val_string->c_str(); }
	constexpr bool isContainer() const { return isMap() || isArray(); }
	constexpr bool isMap() const { return type == T_MAP; }
	constexpr bool isArray() const { return type == T_ARRAY; }
	constexpr bool isNumber() const { return (type & T_NUMBER) != 0; }
	constexpr bool isFloat() const { return type == T_NUMBER_FLOAT; }
	constexpr bool isInt() const { return type == T_NUMBER_INT; }
	constexpr bool isUInt() const { return type == T_NUMBER_UINT; }
	constexpr bool isString() const { return type == T_STRING; }
	constexpr bool isBool() const { return type == T_BOOL; }
	constexpr bool isNull() const { return type == T_NULL; }
	template<int vecSize>
	constexpr bool isVec() const { return isArray() && size() == vecSize; }
	constexpr size_t size() const { return isMap() ? val_map->size() : (isArray() ? val_array->size() : (isString() ? string().size() : 0)); }
	NBTType types() const { return type; }
	NBT() = default;

	~NBT() noexcept {
		destruct();
	}

	NBT(int64_t i)
	{
		val_int = i;
		type = T_NUMBER_INT;
	}
	NBT(uint64_t i)
	{
		val_uint = i;
		type = T_NUMBER_UINT;
	}
	NBT(double i)
	{
		val_float = i;
		type = T_NUMBER_FLOAT;
	}
	NBT(int i)
	{
		val_int = i;
		type = T_NUMBER_INT;
	}
	NBT(float i)
	{
		val_float = (double)i;
		type = T_NUMBER_FLOAT;
	}
	NBT(bool i)
	{
		val_int = i;
		type = T_BOOL;
	}
	template <int vecSize>
	NBT(const glm::vec<vecSize, float>& v)
	{
		buildVec(v);
	}
	int getDigit(char c)
	{
		return c - '0';
	}
	uint64_t toUint64(const Stringo& s)
	{
		uint64_t out = 0;
		uint64_t tens = 1;

		for (int i = s.size() - 1; i >= 0; --i)
		{
			out += tens * getDigit(s[i]);
			tens *= 10;
		}
		return out;
	}
	bool isNumeric(const Stringo& string)
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
	NBT(const Stringo& i)
	{
		if (SUtil::startsWith(i, '#') && isNumeric(i.substr(1)))
		{
			val_uint = toUint64(i.substr(1));
			type = T_NUMBER_UINT;
			return;
		}
		val_string = new Stringo(i);
		type = T_STRING;
	}
	NBT(Stringo&& i)
	{
		if (SUtil::startsWith(i, '#') && isNumeric(i.substr(1)))
		{
			val_uint = toUint64(i.substr(1));
			type = T_NUMBER_UINT;
			return;
		}
		val_string = new Stringo(std::move(i));
		type = T_STRING;
	}
	NBT(const char* i):NBT(Stringo(i)){}
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
  template <int vecSize>
	NBT& operator=(const glm::vec<vecSize, float>& i)
	{
		if (isVec<vecSize>())
			destruct();

		buildVec(i);
		return *this;
	}
	NBT& operator=(const Stringo& i)
	{
		if (type == T_STRING)
			*val_string = i;
		else {
			destruct();
			val_string = new Stringo(i);
			type = T_STRING;
		}
		return *this;
	}
	NBT& operator=(Stringo&& i)
	{
		if (type == T_STRING)
			*val_string = std::move(i);
		else {
			destruct();
			val_string = new Stringo(std::move(i));
			type = T_STRING;
		}
		return *this;
	}

	int invalidCast() const { /*ASSERT(false, "invalid nbt type cast");*/ return 0; }
	NBT& operator=(const char* i)
	{
		return this->operator=(Stringo(i));
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
		return isUInt() ? val_uint : (isInt() ? val_int : invalidCast());
	}

	operator glm::vec2() const
	{
		ASSERT(isVec<2>(), "invalid cast");
		return glm::vec2(this[0], this[1]);
	}
	operator glm::vec3() const
	{
		ASSERT(isVec<3>(), "invalid cast");
		return glm::vec3(this[0], this[1], this[2]);
	}
	operator glm::vec4() const
	{
		ASSERT(isVec<4>(), "invalid cast");
		return glm::vec4(this[0], this[1], this[2], this[3]);
	}

	operator float() const { return isFloat() ? val_float : (isInt() ? val_int : invalidCast()); }
	operator double() const { return isFloat() ? val_float : (isInt() ? val_int : invalidCast()); }

	//will try to return number, if not number -> zero
	double toNumber() const
	{
		if (isInt())
			return val_int;
		if (isUInt())
			return val_uint;
		if (isFloat())
			return val_float;
		return 0;
	}
	Stringo& string() { return isString() ? *val_string : NBT_INVALID_STRING; }
	const Stringo& string() const { return isString() ? *val_string : NBT_INVALID_STRING; }


	//move
	NBT(NBT&& o) noexcept : type(o.type)
	{
		val_int = o.val_int;
		o.val_int = 0;
		o.type = T_NULL;
	}

	//copy
	NBT(const NBT& o) : type(o.type)
	{
		val_int = o.val_int;
		type = o.type;
		if (o.type == T_MAP)
			val_map = new NBTMap(*o.val_map);
		else if (o.type == T_ARRAY)
			val_array = new NBTVector(*o.val_array);
		else if (o.type == T_STRING)
			val_string = new Stringo(*o.val_string);
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
			val_string = new Stringo(*o.val_string);
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
	void emplace_map(const Stringo& name, Arg&& arg)
	{
		if (!checkForMap())
			return;
		val_map->try_emplace(name, std::forward<Arg>(arg));
	}

	//map access
	NBT& access_map(const Stringo& name)
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
	 const NBT& access_map_const(const Stringo& name) const
	{
		if (!isMap())
		{
			ASSERT(false, "");
			return nullVal();
		}
		return val_map->operator[](std::forward<const Stringo>(name));
	}

	NBT& operator[](const Stringo& name)
	{
		return access_map(name);
	}
	NBT& operator[](const char* name)
	{
		return access_map(Stringo(name));
	}
	const NBT& operator[](const Stringo& name) const
	{
		return access_map_const(name);
	}
	const NBT& operator[](const char* name) const
	{
		return access_map_const(Stringo(name));
	}


	void reserve(int size) { if (checkForArray())val_array->reserve(size); }
	void resize(size_t size){ if (checkForArray())val_array->resize(size); }
	bool exists(const Stringo& key) const
	{
		return isMap() && (val_map->find(key) != val_map->end());
	}

	//array access
	NBT& access_array(int index)
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
	const NBT& access_array_const(int index) const
	{
		if (!isArray())
		{
			ASSERT(false, "");
			return nullVal();
		}
		return val_array->operator[](index);
	}

	NBT& operator[](int index)
	{
		return access_array(index);
	}
	const NBT& operator[](int index) const
	{
		return access_array_const(index);
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

	Stringo dump() const
	{
		return dump(0);
	}

	Stringo dump(int depth) const;

	json toJson() const;
	static NBT fromJson(const json& j);
  
	static void saveToFile(const Stringo& filePath, const NBT& nbt);
	// loads nbt from json file (comments in file are allowed, ignored)
	static bool loadFromFile(const Stringo& filePath, NBT& nbt);

	template <typename Arg>
	void save(const Stringo& name,const Arg& val)
	{
		if (!checkForMap())
			return;
		access_map(name) = val;
	}
	//=================Load new================================
	template <typename Arg0, typename Arg1>
	bool load(const Stringo& key, Arg0& val, Arg1 defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = (Arg0)it->second;
		else val = defaultVal;
		return found;
	}
	template <typename Arg1>
	bool load(const Stringo& key, Stringo& val, Arg1 defaultVal) const
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
	//specialization for same types
	template <typename Arg>
	bool loadSet(const Stringo& key, Arg& val, Arg defaultVal)
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = (Arg)it->second;
		else {
      val = defaultVal;
      val_map[key]=defaultVal;
    }
		return found;
	}
	/*template <>
	bool loadNew(const& key, Stringo& val, const char* defaultVal) const
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
	}*/
	//=================Load new==end===========================

	/*template <typename Arg>
	bool load(const Stringo& key, Arg& val, Arg defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = Arg(it->second);
		else {
			val = defaultVal;
			operator[](key) = val;
		}
		return found;
	}


	template <>
	bool loadSet(const Stringo& key, Stringo& val, Stringo defaultVal)
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = it->second.string();
		else {
			val = defaultVal;
			operator[](key) = val;
		}
		return found;
	}


	// If data exists, val=data, if not, val=default_val
	// if exists -> loads data to val
	// if does not exist -> sets val to default
	template <typename Arg>
	bool load(const Stringo& key, Arg& val, Arg defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}

	*/
	/*template <typename Arg>
	bool loadIfExists(const Stringo& key, Arg& val) const
	{
		if (!isMap()) return false;

		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		val = found?Arg(it->second):defaultVal;
		return found;
	}
	template <>
	bool load(const Stringo& key, Stringo& val, Stringo defaultVal) const
	{
		if (!isMap()) {
			val = defaultVal;
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		val = found ? it->second.string(): defaultVal;
		return found;
	}*/
	template <typename Arg>
	bool loadIfExists(const Stringo& key, Arg& val) const
	{
		if (!isMap()) return false;

		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = (Arg)it->second;
		return found;
	}
	template <>
	bool loadIfExists(const Stringo& key, Stringo& val) const
	{
		if (!isMap()) return false;

		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = it->second.string();
		return found;
	}

	
	// If data exists -> val=data
	template <typename Arg>
	bool load(const Stringo& key, Arg& val) const
	{
		if (!isMap()) {
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found)
			val = (Arg)it->second;
		return found;
	}
	template <>
	bool load(const Stringo& key, Stringo& val) const
	{
		if (!isMap()) {
			return false;
		}
		auto& it = val_map->find(key);
		bool found = it != val_map->end();
		if (found && it->second.isString())
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