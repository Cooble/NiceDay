#include "ndpch.h"
#include "basic_blocks.h"
#include "world/World.h"

//AIR=======================================
BlockAir::BlockAir()
	:Block(BLOCK_AIR)
{
	m_block_connect_group = BIT(BLOCK_GROUP_AIR_BIT);
	m_opacity = 0.05f;
	m_texture_pos = -1;
}
int BlockAir::getTextureOffset(int x, int y, const BlockStruct&)const { return -1; }
bool BlockAir::onNeighbourBlockChange(World* world, int x, int y) const { return false; }


//STONE=======================================

BlockStone::BlockStone()
	:Block(BLOCK_STONE)
{
	m_has_big_texture = true;
	m_texture_pos = { 3,0 };
}

//DIRT========================================
BlockDirt::BlockDirt()
	:Block(BLOCK_DIRT)
{
	m_has_big_texture = true;
	m_texture_pos = { 2,0 };
}



//GOLD========================================
BlockGold::BlockGold()
	:Block(BLOCK_GOLD)
{
	m_has_big_texture = true;
	m_texture_pos = { 1,0 };
}

//ADAMANTITE==================================
BlockAdamantite::BlockAdamantite()
	:Block(BLOCK_ADAMANTITE)
{
	m_has_big_texture = true;
	m_texture_pos = { 0,0 };
}

//PLATFORM====================================
BlockPlatform::BlockPlatform()
	:Block(BLOCK_PLATFORM)
{
	m_block_connect_group = BIT(BLOCK_GROUP_PLATFORM_BIT);
	m_texture_pos = { 0,15 };
}
bool BlockPlatform::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastCorner = block.corner;

	bool leftPlat = isInGroup(world, x - 1, y, m_block_connect_group)|| world->isAir(x - 1, y);
	bool rightPlat = isInGroup(world, x + 1, y, m_block_connect_group)|| world->isAir(x + 1, y);


	if (leftPlat && rightPlat)
	{
		block.corner = half_int(1, 15).i;
	}
	else if (leftPlat)
	{
		block.corner = half_int(3, 15).i;
	}
	else if (rightPlat)
	{
		block.corner = half_int(2, 15).i;
	}
	else
	{
		block.corner = half_int(0, 15).i;
	}


	return lastCorner != block.corner;//we have a change (or not)

}

int BlockPlatform::getCornerOffset(int x, int y, const BlockStruct&) const { return BLOCK_STATE_FULL; }

int BlockPlatform::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return s.corner;//use corner as block state
}

//GRASS=======================================

BlockGrass::BlockGrass()
	:Block(BLOCK_GRASS)
{
	m_texture_pos = { 0,0 };
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
}

int BlockGrass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return s.metadata;
}
bool BlockGrass::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastid = block.id;
	int lastCorner = block.corner;
	Block::onNeighbourBlockChange(world, x, y);//update corner state
	if (block.corner == BLOCK_STATE_FULL
		|| block.corner == BLOCK_STATE_BOLD_LINE_DOWN
		|| block.corner == BLOCK_STATE_BOLD_LINE_LEFT
		|| block.corner == BLOCK_STATE_BOLD_LINE_RIGHT
		|| block.corner == BLOCK_STATE_CORNER_DOWN_LEFT
		|| block.corner == BLOCK_STATE_CORNER_DOWN_RIGHT
		|| block.corner == BLOCK_STATE_THIN_LINE_END_DOWN
		|| block.corner == BLOCK_STATE_THIN_LINE_VERTICAL)
		block.id = BLOCK_DIRT;
	else if (block.corner == BLOCK_STATE_CORNER_UP_LEFT
		|| block.corner == BLOCK_STATE_THIN_LINE_END_LEFT)
		block.metadata = half_int(std::rand()%2,12).i;
	else if (block.corner == BLOCK_STATE_CORNER_UP_RIGHT
		|| block.corner == BLOCK_STATE_THIN_LINE_END_RIGHT)
		block.metadata = half_int(2 + std::rand() % 2, 12).i;
	else
		block.metadata = half_int(x&3, 13).i;

	return lastCorner != block.corner || lastid != block.id;//we have a change (or not)

}


