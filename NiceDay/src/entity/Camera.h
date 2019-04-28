#pragma once
#include <glm/vec2.hpp>
class Camera
{
private:
	glm::vec2 position;
	
public:
	Camera();
	~Camera();

	inline const glm::vec2 getPosition() const { return position; };
	inline void setPosition(glm::vec2 vec) { position = vec; }
};

