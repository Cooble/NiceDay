#include "ndpch.h"
#include "Group.h"
#include "BatchRenderer2D.h"

void Group::render(BatchRenderer2D& renderer)
{
	renderer.push(m_transform);
	for (auto r : m_renderables)
		renderer.submit(*r);
	renderer.pop();

}

