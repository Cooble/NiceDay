#pragma once
#include <glm/vec2.hpp>

namespace nd {
// unique identifier of each audio stream of Sounder
typedef int64_t SoundID;

static const float invalidSpatial = std::numeric_limits<float>::max() - 1;

struct SpatialData
{
	// location of sound source
	glm::vec2 pos = {invalidSpatial, 0};
	// x means distance to which sound will have full volume
	// y means distance after which no sound will be heard
	glm::vec2 maxDistances;
	bool isValid() const { return pos.x != invalidSpatial; }
};
}
