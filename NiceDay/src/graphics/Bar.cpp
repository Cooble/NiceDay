#include "ndpch.h"
#include "Bar.h"
#include "graphics/BatchRenderer2D.h"
#include "Texture.h"
#include "glm/gtx/component_wise.hpp"

Bar::Bar(const Sprite& back, const Sprite& fore): m_back_sprite(back), m_fore_sprite(fore)
{
	m_full_x_width = m_fore_sprite.getSize().x;
	m_full_u_width = m_fore_sprite.getUV().uv[1].x- fore.getUV().uv[0].x;
}

Bar Bar::buildDefault(const glm::vec2& pos)
{
	static SpriteSheetResource res(Texture::create(
		TextureInfo("res/images/gui.png")
		.filterMode(TextureFilterMode::NEAREST)
		.format(TextureFormat::RGBA)), 2, 1);
	auto back = Sprite(&res);
	back.setSpriteIndex(1, 0);
	back.setPosition(glm::vec3(pos.x, pos.y, 0));
	back.setSize(glm::vec2(2, 1));

	auto fore = Sprite(&res);
	fore.setSpriteIndex(0, 0);
	fore.setPosition(glm::vec3(pos.x, pos.y, 0));
	fore.setSize(glm::vec2(2, 1));

	return Bar(back, fore);
}


void Bar::setValue(float v)
{
	if(v!=m_value)
	{
		m_value = v;
		m_fore_sprite.setSize(glm::vec2(m_full_x_width*m_value, m_fore_sprite.getSize().y));
		float lastVSize = m_fore_sprite.getUV().uv[2].y-m_fore_sprite.getUV().uv[0].y;
		m_fore_sprite.getUV().setSize(glm::vec2(m_full_u_width*m_value, lastVSize));
	}
}

void Bar::render(BatchRenderer2D& renderer)
{
	if (!m_enabled)
		return;
	renderer.submit(m_back_sprite);
	renderer.submit(m_fore_sprite);
}
