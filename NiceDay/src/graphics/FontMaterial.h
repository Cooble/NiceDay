﻿#pragma once
#include "graphics/API/Texture.h"
#include "font/FontParser.h"

typedef int FontMaterialID;
struct FontMaterial
{
private:
	inline static int currentID;
public:
	const FontMaterialID id;
	FontMaterial();

	Texture* texture;
	Font* font;
	
	glm::vec4 color;
	glm::vec4 border_color;
	glm::vec2 thickness;
	
};
