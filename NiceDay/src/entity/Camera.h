#pragma once
#include <glm/vec2.hpp>
#include "world/ChunkLoader.h"

class Camera : public IChunkLoaderEntity
{
protected:
	//worldpos
	glm::vec2 m_position;

	//size of view
	glm::vec2 m_dimension;

	int m_chunk_radius;

public:
	Camera();
	virtual ~Camera() = default;

	inline const glm::vec2& getPosition() const override;
	inline int getChunkRadius() const override;

	inline void setChunkRadius(int rad) { m_chunk_radius = rad; }

	inline void setPosition(const glm::vec2& vec) { m_position = vec; }


	glm::vec2& getDimension();
};

