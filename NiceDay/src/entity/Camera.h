#pragma once
#include <glm/vec2.hpp>
#include "world/ChunkLoader.h"
#include "world/LightCalculator.h"

class Camera : public IChunkLoaderEntity, public LightSource
{
protected:
	//worldpos
	glm::vec2 m_position;

	//size of view
	glm::vec2 m_dimension;

	int m_chunk_radius;

public:
	float m_light_intensity = 1.5f;
	Camera();
	virtual ~Camera() = default;

	std::pair<int, int> getLightPosition() const override
	{
		return std::make_pair((int)m_position.x, (int)m_position.y);
	}
	inline float getIntensity() const override { return m_light_intensity; }
	inline const glm::vec2& getPosition() const override;
	inline int getChunkRadius() const override;

	inline void setChunkRadius(int rad) { m_chunk_radius = rad; }

	inline void setPosition(const glm::vec2& vec) { m_position = vec; }


	glm::vec2& getDimension();
};

