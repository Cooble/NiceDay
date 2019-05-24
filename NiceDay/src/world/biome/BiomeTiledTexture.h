#pragma once
#include "Biome.h"

class BiomeTiledTexture :public Biome
{
protected:
	BiomeTiledTexture(int id, const std::string& texture_path);
	void updateSprites(World* m_world, Camera* m_camera) override;
};