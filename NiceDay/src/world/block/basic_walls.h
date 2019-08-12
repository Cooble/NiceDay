#pragma once
#include "Block.h"

class WallAir :public Wall
{
public:
	WallAir();
	inline void onTextureLoaded(const TextureAtlas& atlas) override{}
	int getTextureOffset(int wx, int wy, const BlockStruct&) const override;
	int getCornerOffset(int wx, int wy, const BlockStruct&) const override;

	UUID_STRING("air")
};
class WallDirt :public Wall
{
public:
	WallDirt();
	UUID_STRING("dirt")
};
class WallStone :public Wall
{
public:
	WallStone();
	UUID_STRING("stone")
};
class WallGlass :public Wall
{
public:
	WallGlass();
	UUID_STRING("glass")
	int getCornerOffset(int wx, int wy, const BlockStruct&) const override;
};