#pragma once
#include "Sprite.h"

class Bar:public IBatchRenderable2D
{
private:
	float m_full_x_width;
	float m_full_u_width;

	float m_value;
	Sprite m_back_sprite;
	Sprite m_fore_sprite;
	bool m_enabled=true;

public:
	Bar(const Sprite& back,const Sprite& fore);
	Bar() = default;
	static Bar buildDefault(const vec2& pos = glm::vec2(0, 0));

	void setValue(float v);

	inline void setEnabled(bool enabled) { m_enabled = enabled; }

	void render(BatchRenderer2D& renderer) override;

};
