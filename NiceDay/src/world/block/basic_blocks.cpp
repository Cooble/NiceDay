#include "ndpch.h"
#include "basic_blocks.h"
#include "world/World.h"
#include "block_datas.h"
#include "entity/Camera.h"

//AIR=======================================
BlockAir::BlockAir()
	:Block(BLOCK_AIR)
{
	m_block_connect_group = BIT(BLOCK_GROUP_AIR_BIT);
	m_opacity = 1;
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
	m_opacity = 0.05f;
}
bool BlockPlatform::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastMeta = block.block_metadata;

	bool leftPlat = isInGroup(world, x - 1, y, m_block_connect_group) || world->isAir(x - 1, y);
	bool rightPlat = isInGroup(world, x + 1, y, m_block_connect_group) || world->isAir(x + 1, y);


	if (leftPlat && rightPlat)
	{
		block.block_metadata = half_int(1, 15);
	}
	else if (leftPlat)
	{
		block.block_metadata = half_int(3, 15);
	}
	else if (rightPlat)
	{
		block.block_metadata = half_int(2, 15);
	}
	else
	{
		block.block_metadata = half_int(0, 15);
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
	bool custombit = (lastCorner&BIT(4)) != 0;//perserve custombit
	block.block_corner &= ~BIT(4);

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
		block.block_metadata = half_int(x&1, 12);
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_RIGHT)
		block.block_metadata = half_int((x&1)+2, 12);
	else
		block.block_metadata = half_int(x & 3, 13);
	block.block_corner |= custombit << 4;//put back custombit
	return lastCorner != block.block_corner || lastid != block.block_id;//we have a change (or not)

}

//GLASS=======================================

BlockGlass::BlockGlass()
	:Block(BLOCK_GLASS)
{
	m_opacity = 1;
	m_texture_pos = { 0,14 };
	m_corner_translate_array = BLOCK_CORNERS_GLASS;

}

int BlockGlass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return (m_texture_pos + half_int(s.block_metadata, 0));
}

bool BlockGlass::onNeighbourBlockChange(World* world, int x, int y) const
{
	auto c = Block::onNeighbourBlockChange(world, x, y);
	BlockStruct& e = world->editBlock(x, y);
	e.block_metadata = std::rand() % 10;
	e.block_metadata &= 3;
	return c;
}


//up 0 left 1 down 2 right 3
BlockTorch::BlockTorch()
: Block(BLOCK_TORCH)
{
	m_opacity = 0;

}

int BlockTorch::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	return half_int(b.block_corner, 11);
}

int BlockTorch::getCornerOffset(int x, int y, const BlockStruct&) const
{
	return 0;
}
bool BlockTorch::isInTorchGroup(World* world, int x, int y) const
{
	int id = BLOCK_AIR;
	auto p = world->getLoadedBlockPointer(x, y);
	if (p)
		id = p->block_id;
	
	return id != BLOCK_TORCH && id != BLOCK_AIR;
}
bool BlockTorch::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& s = world->editBlock(x, y);
	int lastCorner = s.block_corner;
	
	int mask = 0;
	mask |= (isInTorchGroup(world, x - 1, y) & 1) << 0;
	mask |= (isInTorchGroup(world, x, y - 1) & 1) << 1;
	mask |= (isInTorchGroup(world, x + 1, y) & 1) << 2;
	mask |= (!s.isWallFree()) << 3;//can be placed on wall

	if ((BIT(lastCorner)&mask)==0)
	{
		if((mask&BIT(0))!=0)
		{
			s.block_corner = 0;
		}
		else if ((mask&BIT(1)) != 0)
		{
			s.block_corner = 1;
		}
		else if ((mask&BIT(2)) != 0)
		{
			s.block_corner = 2;
		}
		else if ((mask&BIT(3)) != 0)
		{
			s.block_corner = 3;
		}
		else
		{
			world->setBlock(x, y, BLOCK_AIR);//torch cannot float
		}
	}
	return lastCorner != s.block_corner;
}

void BlockTorch::onBlockPlaced(World* w, int x, int y, BlockStruct& b) const
{
	auto c = new Camera();
	c->setPosition(glm::vec2(x,y));
	w->getLightCalculator().registerLight(c);//memory leak as fuck :D
}



