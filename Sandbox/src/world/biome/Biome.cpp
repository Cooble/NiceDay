#include "ndpch.h"
#include "Biome.h"

//todo destruct textures in sprites
Biome::Biome(int id)
:m_id(id),
m_background_light(0)
{
}

void Biome::updateSprites(World* m_world, Camera* m_camera)
{

}

Biome::~Biome()
{
	for(int i = 0;i<m_sprites_size;i++)
	{
		delete m_sprites[i];
	}
}

void Biome::update(World* m_world, Camera* m_camera)
{
	updateSprites(m_world, m_camera);
}

