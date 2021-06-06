#pragma once
#include "ndpch.h"

namespace nd {

// ======= unwrapping string args to StringId
template <typename String, typename... Args>
StringId nd_string_id_unzip(StringId id, String string, Args&&... args)
{
	return nd_string_id_unzip(id.concat(string), std::forward<Args>(args)...);
}

template <typename String>
StringId nd_string_id_unzip(StringId id, String string)
{
	return id.concat(string);
}

inline StringId nd_string_id_unzip(StringId id)
{
	return id;
}

// =======

// ======= unwrapping string args to string
template <typename String, typename... Args>
std::string nd_string_unzip(std::string& id, String string, Args&&... args)
{
	return nd_string_unzip(std::move(id).append(string), std::forward<Args>(args)...);
}

template <typename String>
std::string nd_string_unzip(std::string& id, String string)
{
	return std::move(id).append(string);
}

inline std::string nd_string_unzip(std::string& id)
{
	return std::move(id);
}

// =======


// translates key created by joining string arguments together
// uses hashing to StringId
//#define ND_TRANSLATE(first, ...) Translator::translate(nd_string_id_unzip(StringId(first),__VA_ARGS__))

// translates key created by joining string arguments together
#define ND_TRANSLATE(first, ...) Translator::translate(nd_string_unzip(std::string(first),__VA_ARGS__))

// Translates keys to their specified words specified in lang file
// It's customary to use "nested.key.structure.btn.play.title" for keys separated by '.'
class Translator
{
private:
	// maybe convert all std::strings to char* in one big buffer (and maybe not)
	//static char* s_buffer=nullptr;
	//static std::unordered_map<Strid, const char*> s_dictionary_buffer;
	// converts disgusting big pile of std::strings to one big chunk of memory of const char*
	//static void bake();

	static std::unordered_map<Strid, std::string> s_dictionary;
	static std::set<std::string> s_unknowns;
	static bool s_unknown_remember;

	static void loadDict(const char* filePath, std::unordered_map<Strid, std::string>& dic);
public:
	// load words from specified file and add them to current dictionary
	static void loadDict(const char* filePath) { return loadDict(filePath, s_dictionary); }

	// appends current unknown entries to file
	static void saveUnknownEntries(const char* filePath);

	static int getNumberOfUnknowns() { return s_unknowns.size(); }

	static void clearDict();

	// lookup of word, if no translation is found returns id
	// if no translation is found and enableUnknownRemember -> this key will be saved
	static const std::string& translate(const std::string& id);

	// returns entry at id, key must be valid! otherwise assert is triggered
	static const std::string& translate(StringId id);

	// return nullptr if word does not exist
	static const std::string* translateTry(StringId id);

	// return key if word does not exist
	static const std::string& translateTryWithKey(const std::string& id);

	// if true any unsuccessful lookup with call to translate(const std::string& id) will be saved
	static void enableUnknownRemember(bool remember) { s_unknown_remember = remember; }
};


struct AppLanguage
{
	// name of lang e.g. "English"
	std::string name;
	// abbreviation of lang e.g. "en"
	std::string abbrev;
};

// Language files registry
// loads Translator with selected language from lang files which are in specified folders
// Note: offers methods to work with Translator
//
// Usage:
// registerLanguage("English","en");
// addLanguageFolder("path/which/contains something_en.lang files/");
// loadLanguage("en");
class AppLanguages
{
private:
	static struct S
	{
		std::string currentLanguage;
		std::vector<AppLanguage> languages;
		// folders which contain lang files
		std::vector<std::string> languageFolders;
	} s_data;

public:
	static void addLanguageFolder(const std::string& folderPath);
	static void registerLanguage(const std::string& language, const std::string& abbrev);
	// searches through all language folders and loads every .lang file to translator associated with specified language
	// such language must be registered first
	// all nag files need to have following structure
	// somethingblabla_<language abbrev>.lang
	static void loadLanguage(const std::string& abbrev);
	static const auto& getLanguages() { return s_data.languages; }
	static std::string getFullLangName(const std::string& abbrev);
	static const std::string& getCurrentLanguage() { return s_data.currentLanguage; }
};
}
