#include "WorldsProvider.h"

static bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}
inline static std::string removeSuffix(const std::string& s,char delimiter)
{
	if (s.find_last_of(delimiter) != s.npos)
		return s.substr(0, s.find_last_of(delimiter));
	return s;

}
inline static std::string removePrefix(const std::string& s, char delimiter)
{
	if (s.find_last_of(delimiter) != s.npos)
		return s.substr(s.find_last_of(delimiter)+1);
	return s;

}
inline static std::string replace(const std::string& s, char src,char dst)
{
	std::string out;
	for (int i = 0; i < s.size(); ++i)
	{
		if (s[i] == src)
			out += dst;
		else out += s[i];
	}
	return out;

}

void WorldsProvider::rescanWorlds()
{
	m_worlds.clear();
	std::string folder = "worlds/";
	
	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(folder))
	{
		if (!std::filesystem::is_regular_file(dirEntry))
			continue;
		std::string txtFile = dirEntry.path().string();
		if (endsWith(txtFile, ".world"))
		{
			
			std::string name = removeSuffix(txtFile,'.');
			name = replace(name,'\\','/');
			name = removePrefix(name, '/');

			m_worlds.push_back({ name,txtFile });
		}
	}
}

void WorldsProvider::deleteWorld(const std::string& name)
{
	for (int i = 0; i < m_worlds.size(); ++i)
	{
		auto& world = m_worlds[i];
		if (world.name == name)
		{
			std::remove(world.path.c_str());
			std::remove((world.path+".entity").c_str());

			m_worlds.erase(m_worlds.begin() + i);
			ND_INFO("deleted {} in: {}", name,world.path);
			return;
		}
	}
}

WorldsProvider& WorldsProvider::get()
{
	static WorldsProvider p;
	return p;
}
