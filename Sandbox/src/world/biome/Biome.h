#pragma once
#include "graphics/Sprite2D.h"
#include "world/Camera.h"
const int BIOME_FOREST		= 0;
const int BIOME_UNDERGROUND = 1;
const int BIOME_DIRT = 2;



class Biome
{
private:
	const int m_id;

protected:
	Sprite2D** m_sprites;
	int m_sprites_size;
	bool m_normal_lighting_enable=false;
	uint8_t m_background_light;
	Biome(int id);
	virtual void updateSprites(World* m_world, Camera* m_camera);
public:
	virtual ~Biome();
	virtual void update(World* m_world, Camera* m_camera);
	inline int getID() const { return m_id; }
	int getBGSpritesSize() { return m_sprites_size; }
	inline Sprite2D** getBGSprites() { return m_sprites; }

	bool hasSkyLighting() const { return m_normal_lighting_enable; }
	virtual uint8_t getBackgroundLight(World* w)
	{
		
			return m_background_light;
	}
	
};
