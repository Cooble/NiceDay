#pragma once
#include "Biome.h"

class BiomeForest :public Biome
{
protected:
	void updateSprites(World* m_world, Camera* m_camera) override;
public:
	BiomeForest();
};
