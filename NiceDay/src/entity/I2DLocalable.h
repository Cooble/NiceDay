#pragma once
#include "glm/vec2.hpp"
class I2DLocalable
{
public:
	virtual ~I2DLocalable() = default;
	virtual const glm::vec2& getPosition() const= 0;
};
