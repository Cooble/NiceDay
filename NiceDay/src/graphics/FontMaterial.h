#pragma once
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
	
};
class FontMatLib
{
	
public:
	// retrives already existing material or loads it
	static FontMaterial* getMaterial(const std::string& name);
};
