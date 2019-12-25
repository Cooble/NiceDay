#pragma once

/******************************************************************************/
/*
  Author  - Ming-Lun "Allen" Chou
  Web     - http://AllenChou.net
  Twitter - @TheAllenChou
*/
/******************************************************************************/

#include <cinttypes>
#include <iostream>     // std::cout, std::ostream, std::ios
#include <fstream>      // std::filebuf

/*
	Makes possible to lookup strings from ids
	Its significantly slower but awesome for debugging

	To obtain string from id call StringIdLookup::getString(id)
 */
#define DEBUG_ID_LOOKUP_ENABLE 1



// disable overflow warnings due to intentional large integer multiplication
#pragma warning (disable: 4307)

// FNV-1a hash
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//-----------------------------------------------------------------------------
constexpr unsigned long long stringIdHashConcat(unsigned long long base, const char* str)
{
	return (*str) ? (stringIdHashConcat((base ^ *str) * 0x100000001b3, str + 1)) : base;
}

constexpr unsigned long long stringIdHash(const char* str)
{
	return stringIdHashConcat(0xcbf29ce484222325, str);
}

//-----------------------------------------------------------------------------
// end FNV-1a hash


// String ID
//-----------------------------------------------------------------------------

class StringId
{
private:
	uint64_t m_data;
public:
	StringId() : StringId(static_cast<uint64_t>(0)) { }
	StringId(uint64_t data) : m_data(data) { }
	StringId(const char* str) : m_data(stringIdHash(str)) { }

	static StringId concat(const StringId& sid, const char* str)
	{
		return sid.concat(str);
	}

	StringId concat(const char* str) const
	{
		return StringId(stringIdHashConcat(m_data, str));
	}

	uint64_t getValue() const { return m_data; }

	uint64_t operator()()const {return m_data;}

};

static std::ostream& operator<<(std::ostream& out, StringId sid)
{
	return out << "sid:" << sid.getValue();
}

static bool operator==(const StringId& lhs, const StringId& rhs)
{
	return lhs.getValue() == rhs.getValue();
}

static bool operator!=(const StringId& lhs, const StringId& rhs)
{
	return lhs.getValue() != rhs.getValue();
}

//-----------------------------------------------------------------------------
// end: String ID


// StringId macros & constants
//-----------------------------------------------------------------------------

class StringIdLookup
{
private:
	StringIdLookup() = default;
	
	static std::unordered_map<uint64_t, std::string> s_strings;
public:
	static void insertString(uint64_t key, const std::string& s);
	static const std::string* getString(uint64_t key);

	static StringId sids(const std::string& s);
	static uint64_t sid(const std::string& s);

	static StringId sids(const char* s);
	static uint64_t sid(const char* s);
};



#if DEBUG_ID_LOOKUP_ENABLE==1
	#define SID(str) (StringIdLookup::sid((str)))
	#define SIDS(str) (StringIdLookup::sids((str)))
#else
	#define SID(str) (StringIdHash(str))
	#define SIDS(str) (StringId(str))
#endif


