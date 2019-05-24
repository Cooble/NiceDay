#pragma once
#include "ndpch.h"
#include "Biome.h"

class BiomeRegistry
{

private:
	std::vector<Biome*> m_biomes;
	BiomeRegistry();

public:
	~BiomeRegistry();

	inline const std::vector<Biome*>& getBiomes() { return m_biomes; }


	static inline BiomeRegistry& get() {
		static BiomeRegistry s_instance;
		return s_instance;
	}
	BiomeRegistry(BiomeRegistry const&) = delete;
	void operator=(BiomeRegistry const&) = delete;
public:
	void registerBiome(Biome*);

	Biome& getBiome(int biome_id);

};
