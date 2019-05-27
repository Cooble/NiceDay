#include "ndpch.h"
#include "basic_blocks.h"
#include "world/World.h"
#include "block_datas.h"

//AIR=======================================
BlockAir::BlockAir()
	:Block(BLOCK_AIR)
{
	m_block_connect_group = BIT(BLOCK_GROUP_AIR_BIT);
	m_opacity = 0.05f;
	m_texture_pos = -1;
}
int BlockAir::getTextureOffset(int x, int y, const BlockStruct&)const { return -1; }

int BlockAir::getCornerOffset(int x, int y, const BlockStruct&) const { return 0; }

bool BlockAir::onNeighbourBlockChange(World* world, int x, int y) const { return false; }


//STONE=======================================

BlockStone::BlockStone()
	:Block(BLOCK_STONE)
{
	m_has_big_texture = true;
	m_texture_pos = { 3,0 };
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
}

//DIRT========================================
BlockDirt::BlockDirt()
	:Block(BLOCK_DIRT)
{
	m_has_big_texture = true;
	m_texture_pos = { 2,0 };
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);


}



//GOLD========================================
BlockGold::BlockGold()
	:Block(BLOCK_GOLD)
{
	m_has_big_texture = true;
	m_texture_pos = { 1,0 };
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_ORE_BIT);


}

//ADAMANTITE==================================
BlockAdamantite::BlockAdamantite()
	:Block(BLOCK_ADAMANTITE)
{
	m_has_big_texture = true;
	m_texture_pos = { 2,1 };
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_ORE_BIT) | BIT(BLOCK_GROUP_DIRT_BIT);


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
	int lastMeta = block.block_metadata;

	bool leftPlat = isInGroup(world, x - 1, y, m_block_connect_group) || world->isAir(x - 1, y);
	bool rightPlat = isInGroup(world, x + 1, y, m_block_connect_group) || world->isAir(x + 1, y);


	if (leftPlat && rightPlat)
	{
		block.block_metadata = half_int(1, 15).i;
	}
	else if (leftPlat)
	{
		block.block_metadata = half_int(3, 15).i;
	}
	else if (rightPlat)
	{
		block.block_metadata = half_int(2, 15).i;
	}
	else
	{
		block.block_metadata = half_int(0, 15).i;
	}


	return lastMeta != block.block_metadata;//we have a change (or not)

}

int BlockPlatform::getCornerOffset(int x, int y, const BlockStruct&) const { return BLOCK_STATE_FULL; }

int BlockPlatform::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return s.block_metadata;//use corner as block state
}

//GRASS=======================================

BlockGrass::BlockGrass()
	:Block(BLOCK_GRASS)
{
	m_texture_pos = { 0,0 };
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
}

int BlockGrass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return s.block_metadata;
}


bool BlockGrass::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastid = block.block_id;
	int lastCorner = block.block_corner;
	Block::onNeighbourBlockChange(world, x, y);//update corner state
	if (block.block_corner == BLOCK_STATE_FULL
		|| block.block_corner == BLOCK_STATE_LINE_DOWN
		|| block.block_corner == BLOCK_STATE_LINE_LEFT
		|| block.block_corner == BLOCK_STATE_LINE_RIGHT
		|| block.block_corner == BLOCK_STATE_CORNER_DOWN_LEFT
		|| block.block_corner == BLOCK_STATE_CORNER_DOWN_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_DOWN
		|| block.block_corner == BLOCK_STATE_LINE_VERTICAL)
	{
		block.block_id = BLOCK_DIRT;
	}
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_LEFT
		|| block.block_corner == BLOCK_STATE_LINE_END_LEFT)
		block.block_metadata = half_int(std::rand() % 2, 12).i;
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_RIGHT)
		block.block_metadata = half_int(2 + std::rand() % 2, 12).i;
	else
		block.block_metadata = half_int(x & 3, 13).i;

	return lastCorner != block.block_corner || lastid != block.block_id;//we have a change (or not)

}

//GLASS=======================================

BlockGlass::BlockGlass()
	:Block(BLOCK_GLASS)
{
	m_opacity = 0.05f;
	m_texture_pos = { 0,14 };
	m_corner_translate_array = BLOCK_CORNERS_DIRT;

}

int BlockGlass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return (m_texture_pos + half_int(s.block_metadata, 0)).i;
}

bool BlockGlass::onNeighbourBlockChange(World* world, int x, int y) const
{
	auto c = Block::onNeighbourBlockChange(world, x, y);
	BlockStruct& e = world->editBlock(x, y);
	e.block_metadata = std::rand() % 10;
	e.block_metadata &= 3;
	return c;
}
