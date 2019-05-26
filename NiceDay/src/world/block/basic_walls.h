#pragma once
#include "Block.h"

class WallAir :public Wall
{
public:
	WallAir();
	int getTextureOffset(int wx, int wy, const BlockStruct&) const override;
	int getCornerOffset(int wx, int wy, const BlockStruct&) const override;

	TO_STRING(WallAir)
};
class WallDirt :public Wall
{
public:
	WallDirt();
	TO_STRING(WallDirt)
};
class WallStone :public Wall
{
public:
	WallStone();
	TO_STRING(WallStone)
};