#include "ndpch.h"
#include "BiomeTiledTexture.h"
#include "Game.h"
#include "world/WorldRenderManager.h"

BiomeTiledTexture::BiomeTiledTexture(int id, const std::string& texture_path)
	:Biome(id)
{
	m_sprites_size = 1;
	m_sprites = new Sprite2D*[m_sprites_size];

	TextureInfo info(texture_path);
	info.wrap_mode_s = GL_REPEAT;
	info.wrap_mode_t = GL_REPEAT;
	m_sprites[0] = new Sprite2D(new Texture(info));
	m_sprites[0]->setPosition(glm::vec2(-1, -1));
	m_sprites[0]->setScale(glm::vec2(2, 2));
}

void BiomeTiledTexture::updateSprites(World* m_world, Camera* m_camera)
{
	using namespace glm;
	vec2 screenDim = vec2(Game::get().getWindow()->getWidth(), Game::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	vec2 upperScreen = m_camera->getPosition()+  ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	screenDim = upperScreen - lowerScreen;

	Sprite2D& s = *m_sprites[0];

	auto texDim = vec2(s.getTexture().getWidth(), s.getTexture().getHeight());
	auto pos = vec2(
		m_world->getInfo().chunk_width / 2 * WORLD_CHUNK_SIZE,
		-1 * 2 + (float)s.getTexture().getHeight() / BLOCK_PIXEL_SIZE / 3 + m_world->getInfo().terrain_level);
	//pos = pos - (m_camera->getPosition()-pos);
	vec2 meshLower = pos;
	vec2 meshUpper = pos + texDim / (float)BLOCK_PIXEL_SIZE;
	vec2 meshDim = meshUpper - meshLower;

	vec2 delta = m_camera->getPosition()*2.0f - (meshLower + meshUpper) / 2.0f;//delta of centers
	//delta = delta / (3.0f - 1);//we dont want wall to move slower than blocks

	auto transl = delta / meshDim;
	auto scal = screenDim / meshDim;
	mat4 t(1.0f);
	t = glm::translate(t, vec3(transl.x, transl.y, 0));
	t = glm::scale(t, vec3(scal.x, scal.y, 0));
	s.setUVMatrix(t);
}
