#include "ndpch.h"
#include "basic_blocks.h"

//AIR=======================================
BlockAir::BlockAir()
	:Block(BLOCK_AIR)
{}

int BlockAir::getTextureOffset(int metadata) const
{
	return -1;
}

//STONE=======================================

BlockStone::BlockStone()
	:Block(BLOCK_STONE)
{}

int BlockStone::getTextureOffset(int metadata) const
{
	return 3;
}

//DIRT========================================
BlockDirt::BlockDirt()
	:Block(BLOCK_DIRT)
{}

int BlockDirt::getTextureOffset(int metadata) const
{
	return 2;
}

//GOLD========================================
BlockGold::BlockGold()
	:Block(BLOCK_GOLD)
{}

int BlockGold::getTextureOffset(int metadata) const
{
	return 1;
}

BlockAdamantite::BlockAdamantite()
	:Block(BLOCK_ADAMANTITE)
{}

int BlockAdamantite::getTextureOffset(int metadata) const
{
	return 0;
}
