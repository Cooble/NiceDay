#pragma once
#include "IBatchRenderable2D.h"
#include "Renderable2D.h"

class Group:public IBatchRenderable2D
{
private:
	glm::mat4 m_transform;
	std::vector<Renderable2D*> m_renderables;
public:
	void render(BatchRenderer2D& renderer) override;
	inline void setTransform(const glm::mat4& t){m_transform = t;}
	
};
