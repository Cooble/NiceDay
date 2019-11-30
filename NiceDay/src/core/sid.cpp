#include "ndpch.h"
#include "sid.h"


std::unordered_map<uint64_t, std::string> StringIdLookup::s_strings;

void StringIdLookup::insertString(uint64_t key, const std::string& s)
{
	s_strings[key] = s;
}

const std::string* StringIdLookup::getString(uint64_t key)
{
	auto i = s_strings.find(key);
	if (i == s_strings.end())
		return nullptr;
	return &i->second;
}

StringId StringIdLookup::sid(const std::string& s)
{
	StringId e(s.c_str());
	StringIdLookup::insertString(e(), s);
	return e;
}

uint64_t StringIdLookup::sid_val(const std::string& s)
{
	StringId e(s.c_str());
	StringIdLookup::insertString(e(), s);
	return e();
}

StringId StringIdLookup::sid(const char* s)
{
	StringId e(s);
	StringIdLookup::insertString(e(), std::string(s));
	return e;
}

uint64_t StringIdLookup::sid_val(const char* s)
{
	StringId e(s);
	StringIdLookup::insertString(e(), std::string(s));
	return e();
}
