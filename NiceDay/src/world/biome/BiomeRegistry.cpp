#include "ndpch.h"
#include "BiomeRegistry.h"
BiomeRegistry::BiomeRegistry()= default;

void BiomeRegistry::registerBiome(Biome* b)
{
	m_biomes.resize(b->getID() + 1);
	m_biomes[b->getID()] = b;
}

Biome& BiomeRegistry::getBiome(int biome_id)
{
	ASSERT(m_biomes.size() > biome_id && biome_id >=0, "Invalid biome id");
	return *m_biomes[biome_id];
}


BiomeRegistry::~BiomeRegistry()
{
	for (Biome* b : m_biomes)
		delete b;
}