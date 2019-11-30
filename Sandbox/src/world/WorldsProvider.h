#pragma once
#include "ndpch.h"

struct WorldInfoData
{
	std::string name;
	std::string path;
};
class WorldsProvider
{
private:
	WorldsProvider() = default;
	std::vector<WorldInfoData> m_worlds;
	
public:
	inline const std::vector<WorldInfoData>& getAvailableWorlds() const { return m_worlds; }

	void rescanWorlds();
	void deleteWorld(const std::string& name);

	static WorldsProvider& get();
	
};
