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

void Sprite::setSpriteIndex(uint32_t x, uint32_t y)
{
	float xPiece = 1.f / m_resource->getIconsWidth();
	float yPiece = 1.f / m_resource->getIconsHeight();

	m_uv_quad = UVQuad(glm::vec2(xPiece * x, yPiece * y), glm::vec2(xPiece, yPiece));
}

void Sprite::render(BatchRenderer2D& r)
{
	if (m_enabled)
		r.submit(*this);
}
