#pragma once
#include "ndpch.h"
#include "world/ChunkLoader.h"
#include "world/LightCalculator.h"

class Camera : public IChunkLoaderEntity, public LightSource
{
protected:
	//worldpos
	glm::vec2 m_position;

	half_int m_chunk_radius;

public:
	uint8_t m_light_intensity = 23;
	Camera();
	virtual ~Camera() = default;

	std::pair<int, int> getLightPosition() const override
	{
		return std::make_pair((int)m_position.x, (int)m_position.y);
	}
	inline uint8_t getIntensity() const override { return m_light_intensity; }
	inline const glm::vec2& getPosition() const override;
	inline half_int getChunkRadius() const override;

	inline void setChunkRadius(half_int rad) { m_chunk_radius = rad; }

	inline void setPosition(const glm::vec2& vec) { m_position = vec; }
};

