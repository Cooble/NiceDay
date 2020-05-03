#pragma once
#include "Block.h"
#include "graphics/BlockTextureAtlas.h"

class WallAir :public Wall
{
public:
	WallAir();
	void onTextureLoaded(const BlockTextureAtlas& atlas) override{}
	int getTextureOffset(int wx, int wy, const BlockStruct&) const override;
	int getCornerOffset(int wx, int wy, const BlockStruct&) const override;
};
class WallGlass :public Wall
{
public:
	WallGlass();
	int getCornerOffset(int wx, int wy, const BlockStruct&) const override;
};