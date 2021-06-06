#pragma once
#include "ndpch.h"
#include "graphics/Sprite.h"

class Bar:public nd::IBatchRenderable2D
{
private:
	float m_full_x_width;
	float m_full_u_width;

	float m_value;
	nd::Sprite m_back_sprite;
	nd::Sprite m_fore_sprite;
	bool m_enabled=true;

public:
	Bar(const nd::Sprite& back,const nd::Sprite& fore);
	Bar() = default;
	static Bar buildDefault(const glm::vec2& pos = glm::vec2(0, 0));

	void setValue(float v);

	inline void setEnabled(bool enabled) { m_enabled = enabled; }

	void render(nd::BatchRenderer2D& renderer) override;
};
