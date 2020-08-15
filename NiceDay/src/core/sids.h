#pragma once

/******************************************************************************/
/*
  Author  - Ming-Lun "Allen" Chou
  Web     - http://AllenChou.net
  Twitter - @TheAllenChou
*/
/******************************************************************************/

#include <cinttypes>


/*
	Makes possible to lookup strings from ids
	Its significantly slower but awesome for debugging

	To obtain string from id call StringIdLookup::getString(id)
 */
#define DEBUG_ID_LOOKUP_ENABLE 0


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
constexpr unsigned long long stringIdHash(const std::string& str)
{
	return stringIdHashConcat(0xcbf29ce484222325, str.c_str());
}

//-----------------------------------------------------------------------------
// end FNV-1a hash


typedef uint64_t Strid;
// String ID
//-----------------------------------------------------------------------------
class StringId
{
private:
	Strid m_data;
public:
	StringId() : StringId(static_cast<Strid>(0)) { }
	StringId(Strid data) : m_data(data) { }
	StringId(const char* str) : m_data(stringIdHash(str)) { }
	StringId(const std::string& str) : m_data(stringIdHash(str.c_str())) { }

	static StringId concat(const StringId& sid, const char* str)
	{
		return sid.concat(str);
	}

	StringId concat(const char* str) const
	{
		return StringId(stringIdHashConcat(m_data, str));
	}
	StringId concat(char str) const
	{
		return StringId(stringIdHashConcat(m_data, &str));
	}
	StringId concat(const std::string& str) const
	{
		return StringId(stringIdHashConcat(m_data, str.c_str()));
	}

	Strid getValue() const { return m_data; }

	Strid operator()()const {return m_data;}

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
	
	static std::unordered_map<Strid, std::string> s_strings;
public:
	static void insertString(Strid key, const std::string& s);
	static const std::string* getString(Strid key);

	static StringId sids(const std::string& s);
	static Strid sid(const std::string& s);

	static StringId sids(const char* s);
	static Strid sid(const char* s);
};



#if DEBUG_ID_LOOKUP_ENABLE == 1
	#define SID(str) (StringIdLookup::sid((str)))
	#define SIDS(str) (StringIdLookup::sids((str)))
#else
	#define SID(str) (stringIdHash(str))
	#define SIDS(str) (StringId(str))
#endif


