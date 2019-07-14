#include "ndpch.h"
#include "Camera.h"


Camera::Camera()
	:m_position({ 0,0 }),m_chunk_radius(0)
{
	
}

const glm::vec2& Camera::getPosition() const
{
	return m_position;
}

half_int Camera::getChunkRadius() const
{
	return m_chunk_radius;
}


