#include "ndpch.h"
#include "Biome.h"

using namespace nd;

//todo destruct textures in sprites
Biome::Biome(int id)
:m_id(id),
m_background_light(0),
m_sprites_size(0),
m_sky_sprites_size(0),
m_sprites(nullptr),
m_sky_sprites(nullptr)
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
	for (int i = 0; i < m_sky_sprites_size; i++)
	{
		delete m_sky_sprites[i];
	}
}

void Biome::update(World* m_world, Camera* m_camera)
{
	updateSprites(m_world, m_camera);
}

