#include "ndpch.h"
#include "Camera.h"


Camera::Camera()
	:m_position({ 0,0 }),m_dimension({1,1}),m_chunk_radius(0)
{
	
}

const glm::vec2& Camera::getPosition() const
{
	return m_position;
}

int Camera::getChunkRadius() const
{
	return m_chunk_radius;
}


glm::vec2& Camera::getDimension()
{
	return m_dimension;
}
