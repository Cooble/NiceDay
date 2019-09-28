#include "ndpch.h"
#include "BiomeForest.h"
#include "App.h"
#include "world/WorldRenderManager.h"

BiomeForest::BiomeForest()
	:Biome(BIOME_FOREST)
{
	m_background_light = 16;
	m_sprites_size = 3;
	m_normal_lighting_enable = true;
	m_sprites = new Sprite2D*[m_sprites_size];
	for (int i = 0; i < m_sprites_size; i++)
	{
		TextureInfo info(std::string("res/images/bg/forest_") + std::to_string(i) + std::string(".png"));
		info.wrap_mode_s = TextureWrapMode::REPEAT;
		info.wrap_mode_t = TextureWrapMode::CLAMP_TO_BORDER;
		m_sprites[i] = new Sprite2D(Texture::create(info));
		m_sprites[i]->setPosition(glm::vec2(-1, -1));
		m_sprites[i]->setScale(glm::vec2(2, 2));
	}
}

void BiomeForest::updateSprites(World* m_world, Camera* m_camera)
{
	using namespace glm;
	vec2 screenDim = vec2(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	vec2 upperScreen = m_camera->getPosition() + ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	screenDim = upperScreen - lowerScreen;

	for (int i = 0; i < m_sprites_size; i++)
	{
		Sprite2D& s = *m_sprites[i];

		auto texDim = vec2(s.getTexture().getWidth()/1.5 , s.getTexture().getHeight() / 1.5);
		auto pos = vec2(
			m_world->getInfo().chunk_width / 2 * WORLD_CHUNK_SIZE,
			-i * 2 + (float)s.getTexture().getHeight() / BLOCK_PIXEL_SIZE / 3 + m_world->getInfo().terrain_level);
		//pos = pos - (m_camera->getPosition()-pos);
		vec2 meshLower = pos;
		vec2 meshUpper = pos + texDim / (float)BLOCK_PIXEL_SIZE;
		vec2 meshDim = meshUpper - meshLower;

		vec2 delta = m_camera->getPosition() - (meshLower + meshUpper) / 2.0f;//delta of centers
		delta = delta / (3.0f - i);
		delta.y /= 2.0f;//we need smaller change in y

		auto transl = delta / meshDim;

		if (i == 2)
		{
			transl.y += 0.5f;//blocks lower
		}
		auto scal = screenDim / meshDim;
		mat4 t(1.0f);
		t = glm::translate(t, vec3(transl.x, transl.y, 0));
		t = glm::scale(t, vec3(scal.x, scal.y, 0));
		s.setUVMatrix(t);
	}
}
