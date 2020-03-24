#include "ndpch.h"
#include "sids.h"


std::unordered_map<Strid, std::string> StringIdLookup::s_strings;

void StringIdLookup::insertString(Strid key, const std::string& s)
{
	auto f = s_strings.find(key);
	if(f!=s_strings.end()&&f->first!=key)
	{
		ERROR("StringIdClash: \"{}\" with already existing value \"{}\". Same ID: {}", s, f->first, key);
		ASSERT(false, "StringIdClash");
	}
	s_strings[key] = s;
}

const std::string* StringIdLookup::getString(uint64_t key)
{
	auto i = s_strings.find(key);
	if (i == s_strings.end())
		return nullptr;
	return &i->second;
}

StringId StringIdLookup::sids(const std::string& s)
{
	StringId e(s.c_str());
	StringIdLookup::insertString(e(), s);
	return e;
}

Strid StringIdLookup::sid(const std::string& s)
{
	StringId e(s.c_str());
	StringIdLookup::insertString(e(), s);
	return e();
}

StringId StringIdLookup::sids(const char* s)
{
	StringId e(s);
	StringIdLookup::insertString(e(), std::string(s));
	return e;
}

Strid StringIdLookup::sid(const char* s)
{
	StringId e(s);
	StringIdLookup::insertString(e(), std::string(s));
	return e();
}
