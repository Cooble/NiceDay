#pragma once

#include <glm/vec4.hpp>

class Texture;
class Material
{
public:
	struct ColorChannel
	{
		bool enabled=true;
		Texture* texture=nullptr;
		glm::vec4 color={ 0.8f,0.8f,0.8f,1};
	} color;
	struct LightingChannel
	{
		bool enabled=true;
		float ambientEffect=0.05f;
		float diffuseEffect=0.8;
		float reflectivePower=64;
	} lighting;
};
