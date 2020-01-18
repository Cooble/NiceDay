#pragma once
#include "Biome.h"

class BiomeForest :public Biome
{
protected:
	Sprite2D* m_sun;
	Sprite2D* m_moon;
	Sprite2D* m_star;
	void updateSprites(World* m_world, Camera* m_camera) override;
public:
	BiomeForest();
};
