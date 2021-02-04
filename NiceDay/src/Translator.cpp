#include "Translator.h"

#include <fstream>
#include <string>
#include <core/SUtil.h>


#include "core/App.h"
#include "core/NBT.h"
#include "files/FUtil.h"


std::unordered_map<Strid, std::string> Translator::s_dictionary;
std::set<std::string> Translator::s_unknowns;
bool Translator::s_unknown_remember=true;

void Translator::loadDict(const char* filePath, std::unordered_map<Strid, std::string>& dic)
{
	ND_INFO("Loading dictionary: {}", filePath);
	std::string commonPrefix = "";
	std::ifstream file(filePath);
	if (file.is_open()) {
		std::string s;
		while (std::getline(file, s)) {
			if (SUtil::startsWith(s, "->")) {
				commonPrefix = s.substr(2);
				if (!SUtil::endsWith(commonPrefix, '.'))
					commonPrefix += '.';
			}
			else if (SUtil::startsWith(s, "<-")) {
				commonPrefix = "";
			}
			else if (s.empty() || SUtil::startsWith(s, "//") || SUtil::startsWith(s, '#')) {
				continue;
			}
			else {
				auto indexEquuls = s.find_first_of('=');
				auto indexStop = s.find_first_of("\t =");
				if (indexEquuls != std::string::npos) {
					std::string translated = s.substr(indexEquuls + 1);
					std::string src = commonPrefix + s.substr(0, indexStop);
					if (SUtil::startsWith(translated, '{') && translateTry(SID(src)) != nullptr) {
						dic[SID(src)] = *translateTry(SID(src));
					}
					else
						dic[SID(src)] = translated;
				}
			}
		}
	}
	file.close();
}

void Translator::saveUnknownEntries(const char* filePath)
{

	std::unordered_map<Strid, std::string> newDic;
	loadDict(filePath, newDic);
	
	std::ofstream file;
	file.open(filePath, std::ios_base::app);
	bool first = false;
	for(const auto& s:s_unknowns)
		if (newDic.find(SID(s)) == newDic.end()) {//if this file does not contain the entry yet
			if (!first) {
				file << "\n// This is start of automatically generated unknown list:\n";
				first = true;
			}

			file << s << "=?" << s << "?\n";
		}
			
	ND_INFO("Saved unknown dictionary entries to: {}", filePath);
	file.close();
}

void Translator::clearDict()
{
	s_dictionary.clear();
}

const std::string& Translator::translate(const std::string& id)
{
	auto t = translateTry(StringId(id));
	if (t)return *t;
	if(s_unknown_remember)
		s_unknowns.emplace(id);
	return id;
}


const std::string& Translator::translate(StringId id)
{
	ASSERT(translateTry(id()) != nullptr, "Translation not present! for id: {}", id());
	return s_dictionary[id()];
}

const std::string* Translator::translateTry(StringId id)
{
	auto it = s_dictionary.find(id());
	if (it != s_dictionary.end())
		return &(it->second);
	return nullptr;
}

const std::string& Translator::translateTryWithKey(const std::string& id)
{
   auto it = s_dictionary.find(SID(id));
   if (it != s_dictionary.end())
	  return (it->second);
   return id;
}

AppLanguages::S AppLanguages::s_data;

void AppLanguages::addLanguageFolder(const std::string& folderPath)
{
	ASSERT(FUtil::exists(folderPath),"this folder does not exist, isn't that strange?");
	s_data.languageFolders.push_back(folderPath);
}

void AppLanguages::registerLanguage(const std::string& language, const std::string& abbrev)
{
	s_data.languages.push_back({ language,abbrev });
}

void AppLanguages::loadLanguage(const std::string& abbrev)
{
	bool exist = false;
	for (auto& wrap : s_data.languages)
		if (abbrev == wrap.abbrev) {
			exist = true;
			break;
		}
	if (!exist) {
		ND_ERROR("Cannot load language which was not rregistered");
		return;
	}
	
	Translator::clearDict();
	for (auto & folder: s_data.languageFolders)
	{
		auto list = FUtil::fileList(folder,FUtil::FileSearchFlags_Recursive);
		for(auto& file:list)
		{
			if(SUtil::endsWith(file,".lang"))
			{
				auto underIdx = file.find_last_of('_');
				if(underIdx!=std::string::npos)
				{
					auto lang = file.substr(underIdx + 1, file.find_last_of('.')- underIdx - 1);
					if (abbrev == lang)
						Translator::loadDict(file.c_str());
				}
			}
		}
	}
	s_data.currentLanguage = abbrev;
	App::get().getSettings()["language"] = abbrev;
}

std::string AppLanguages::getFullLangName(const std::string& abbrev)
{
	for (auto & l : s_data.languages)
		if (l.abbrev == abbrev)
			return l.name;
	return "";
}
