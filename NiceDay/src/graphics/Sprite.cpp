#include "ndpch.h"
#include "Sprite.h"
#include "BatchRenderer2D.h"

SpriteSheetResource::SpriteSheetResource(Texture* t, uint32_t width, uint32_t height)
	: m_texture(t), m_width_icons(width), m_height_icons(height)
{
}

Sprite::Sprite(SpriteSheetResource* r)
	: m_resource(r),
	  m_uv_quad(glm::vec2(0, 0),
	            glm::vec2(1, 1)),
	  m_position(vec3(0.f, 0, 0)),
	  m_size(vec2(1.f, 1)),
	  m_enabled(true)
{
}

Sprite::Sprite()
	: m_resource(nullptr),
	  m_enabled(false)
{
}

void Sprite::setSpriteIndex(uint32_t u, uint32_t v, bool horizontalFlip, bool verticalFlip, bool rotate90)
{
	float xPiece = 1.f / m_resource->getIconsWidth();
	float yPiece = 1.f / m_resource->getIconsHeight();

	m_uv_quad = UVQuad::build(glm::vec2(xPiece * u, yPiece * v), glm::vec2(xPiece, yPiece), horizontalFlip,
	                          verticalFlip,rotate90);
}

void Sprite::render(BatchRenderer2D& r)
{
	if (m_enabled)
		r.submit(*this);
}

Animation::Animation(SpriteSheetResource* r, std::initializer_list<int> uvs, bool repeatSaw, bool horizontalFlip, bool verticalFlip)
	: Sprite(r),
	  m_repeat_saw(repeatSaw),
	  m_horizontalFlip(horizontalFlip),
	  m_verticalFlip(verticalFlip)
{
	if (uvs.size() == 0)
		return;
	m_indexes.resize(uvs.size());
	int i = 0;
	for (int index : uvs)
		m_indexes[i++] = index;
}

void Animation::reset()
{
	m_increase_index = 1;
	m_current_index = -1;
}

void Animation::updateAfterFlip()
{
	setSpriteIndex(m_indexes[m_current_index], 0, m_horizontalFlip, m_verticalFlip);
}

void Animation::setSpriteFrame(int u, int v)
{
	setSpriteIndex(u, v, m_horizontalFlip, m_verticalFlip);
}

void Animation::nextFrame()
{
	m_current_index += m_increase_index;

	setSpriteIndex(m_indexes[m_current_index], 0, m_horizontalFlip, m_verticalFlip);


	if (m_current_index == m_indexes.size() - 1)
	{
		if (m_repeat_saw)
			m_current_index = -1;
		else
			m_increase_index = -1;
	}
	else if (m_current_index == 0)
		m_increase_index = 1;
}
