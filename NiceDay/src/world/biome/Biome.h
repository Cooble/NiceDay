#pragma once
#include "graphics/Sprite2D.h"
#include "entity/Camera.h"
const int BIOME_FOREST		= 0;
const int BIOME_UNDERGROUND = 1;
const int BIOME_DIRT = 2;

constexpr uint8_t max_light=16;
constexpr uint8_t min_light = 4;

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

	bool hasDynamicLighting() const { return m_normal_lighting_enable; }
	virtual uint8_t getBackgroundLight(World* w)
	{
		if(m_normal_lighting_enable)
		{
			auto time = w->getWorldTime();
			float dayRatio = time.hours() / 24-0.25f;
			return clamp((int)(sin(dayRatio * 2 * 3.14159)*(max_light-min_light)+min_light),(int)min_light,(int)max_light);
		}
		else 
			return m_background_light;
	}
	
};
