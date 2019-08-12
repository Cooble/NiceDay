#include "ndpch.h"
#include "basic_blocks.h"
#include "world/World.h"
#include "block_datas.h"
#include "entity/Camera.h"
#include "graphics/TextureAtlas.h"

//AIR=======================================
BlockAir::BlockAir()
	: Block(BLOCK_AIR)
{
	setNoCollisionBox();
	m_block_connect_group = BIT(BLOCK_GROUP_AIR_BIT);
	m_opacity = 1;
	m_texture_pos = -1;
}

int BlockAir::getTextureOffset(int x, int y, const BlockStruct&) const { return -1; }

int BlockAir::getCornerOffset(int x, int y, const BlockStruct&) const { return 0; }

bool BlockAir::onNeighbourBlockChange(World* world, int x, int y) const { return false; }


//STONE=======================================

BlockStone::BlockStone()
	: Block(BLOCK_STONE)
{
	m_has_big_texture = true;
	m_texture_pos = {3, 0};
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
}


//DIRT========================================
BlockDirt::BlockDirt()
	: Block(BLOCK_DIRT)
{
	m_has_big_texture = true;
	m_texture_pos = {2, 0};
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
}


//GOLD========================================
BlockGold::BlockGold()
	: Block(BLOCK_GOLD)
{
	m_has_big_texture = true;
	m_texture_pos = {1, 0};
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_ORE_BIT);
}

//ADAMANTITE==================================
BlockAdamantite::BlockAdamantite()
	: Block(BLOCK_ADAMANTITE)
{
	m_has_big_texture = true;
	m_texture_pos = {2, 1};
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
	m_block_connect_group = BIT(BLOCK_GROUP_ORE_BIT) | BIT(BLOCK_GROUP_DIRT_BIT);
}

//PLATFORM====================================
BlockPlatform::BlockPlatform()
	: Block(BLOCK_PLATFORM)
{
	m_block_connect_group = BIT(BLOCK_GROUP_PLATFORM_BIT);
	m_texture_pos = {0, 15};
	m_opacity = 1;
	m_collision_box = &BLOCK_BOUNDS_PLATFORM;
	m_collision_box_size = 1;
}

bool BlockPlatform::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastMeta = block.block_metadata;

	int idLeft = world->getBlock(x - 1, y).block_id;
	int idRight = world->getBlock(x + 1, y).block_id;

	bool leftPlat = getID() == idLeft;
	bool rightPlat = getID() == idRight;

	bool isLeftAir = idLeft == BLOCK_AIR;
	bool isRightAir = idRight == BLOCK_AIR;


	if (leftPlat && rightPlat)
	{
		block.block_metadata = half_int(1, 0);
	}
	else if (!rightPlat && !leftPlat && !isLeftAir && !isRightAir)
	{
		block.block_metadata = half_int(0, 0);
	}
	else if (isLeftAir && isRightAir)
	{
		block.block_metadata = half_int(6, 0);
	}
	else if (isLeftAir && rightPlat)
	{
		block.block_metadata = half_int(4, 0);
	}
	else if (isRightAir && leftPlat)
	{
		block.block_metadata = half_int(5, 0);
	}
	else if (!isLeftAir&&!leftPlat)
	{
		block.block_metadata = half_int(2, 0);
	}
	else
	{
		block.block_metadata = half_int(3, 0);
	}
	return lastMeta != block.block_metadata; //we have a change (or not)
}

int BlockPlatform::getCornerOffset(int x, int y, const BlockStruct&) const { return BLOCK_STATE_FULL; }

int BlockPlatform::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_pos + (half_int)s.block_metadata; //use corner as block state
}

//GRASS=======================================

BlockGrass::BlockGrass()
	: Block(BLOCK_GRASS)
{
	m_texture_pos = {0, 0};
	m_block_connect_group = BIT(BLOCK_GROUP_DIRT_BIT);
	m_corner_translate_array = BLOCK_CORNERS_DIRT;
}

int BlockGrass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_pos + (half_int)s.block_metadata;
}


bool BlockGrass::onNeighbourBlockChange(World* world, int x, int y) const
{
	BlockStruct& block = world->editBlock(x, y);
	int lastid = block.block_id;
	int lastCorner = block.block_corner;
	bool custombit = (lastCorner & BIT(4)); //perserve custombit
	block.block_corner &= ~BIT(4);

	Block::onNeighbourBlockChange(world, x, y); //update corner state
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
		block.block_metadata = half_int(x & 1, 0);
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_RIGHT)
		block.block_metadata = half_int((x & 1) + 2, 0);
	else
		block.block_metadata = half_int(x & 3, 1);
	block.block_corner |= custombit << 4; //put back custombit
	return lastCorner != block.block_corner || lastid != block.block_id; //we have a change (or not)
}

//GLASS=======================================

BlockGlass::BlockGlass()
	: Block(BLOCK_GLASS)
{
	m_opacity = 1;
	m_texture_pos = {0, 14};
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
	setNoCollisionBox();
	m_opacity = 0;
	m_light_src = 16;
}

int BlockTorch::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	return m_texture_pos + half_int(b.block_corner, 0);
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
	mask |= (!s.isWallFree()) << 3; //can be placed on wall

	if ((BIT(lastCorner) & mask) == 0)
	{
		if ((mask & BIT(0)) != 0)
		{
			s.block_corner = 0;
		}
		else if ((mask & BIT(1)) != 0)
		{
			s.block_corner = 1;
		}
		else if ((mask & BIT(2)) != 0)
		{
			s.block_corner = 2;
		}
		else if ((mask & BIT(3)) != 0)
		{
			s.block_corner = 3;
		}
		else
		{
			world->setBlock(x, y, BLOCK_AIR); //torch cannot float
		}
	}
	return lastCorner != s.block_corner;
}

BlockDoor::BlockDoor(int id)
	: MultiBlock(id)
{
	m_opacity = 1;
	m_collision_box = &BLOCK_BOUNDS_DOOR;
	m_collision_box_size = 1;
}

void BlockDoor::onBlockClicked(World* w, WorldEntity* e, int x, int y, BlockStruct& curBlok) const
{
	quarter_int offset = curBlok.block_metadata;

	int mainBlockX = x - offset.x;
	int mainBlockY = y - offset.y;
	if (getID() == BLOCK_DOOR_CLOSE)
	{
		BlockStruct bnew = 0;
		bnew.block_id = BLOCK_DOOR_OPEN;

		if (e != nullptr && e->getPosition().x > x) //open to left
		{
			bnew.block_metadata = quarter_int(0, 0, 1, 0);
			for (int yyy = 0; yyy < 3; ++yyy)
			{
				if (!w->isAir(mainBlockX - 1, mainBlockY + yyy))
					return;
			}
			w->setBlock(mainBlockX - 1, mainBlockY, bnew);
		}
		else
		{
			//open to right
			bnew.block_metadata = quarter_int(0, 0, 0, 0);
			for (int yyy = 0; yyy < 3; ++yyy)
			{
				if (!w->isAir(mainBlockX + 1, mainBlockY + yyy))
					return;
			}
			w->setBlock(mainBlockX, mainBlockY, bnew);
		}
	}
	else
	{
		BlockStruct bnew = 0;
		bnew.block_id = BLOCK_DOOR_CLOSE;
		bool z = quarter_int(curBlok.block_metadata).z == 1;
		BlockRegistry::get().getBlock(BLOCK_DOOR_OPEN).onBlockDestroyed(w, e, mainBlockX, mainBlockY,
		                                                                w->editBlock(mainBlockX, mainBlockY));
		w->setBlock(mainBlockX + (z ? 1 : 0), mainBlockY, bnew);
	}
}

BlockDoorOpen::BlockDoorOpen()
	: BlockDoor(BLOCK_DOOR_OPEN)
{
	m_height = 3;
	m_width = 2;
	m_texture_pos = {1, 8};
	m_collision_box_size = 0;
}

void BlockDoorOpen::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("block/door") + half_int(1, 0);
}

BlockDoorClose::BlockDoorClose()
	: BlockDoor(BLOCK_DOOR_CLOSE)
{
	m_height = 3;
	m_width = 1;
	m_texture_pos = {0, 8};
}

bool BlockDoorClose::canBePlaced(World* w, int x, int y) const
{
	if (!MultiBlock::canBePlaced(w, x, y))
		return false;
	auto& str = w->getBlock(x, y - 1);
	auto& b = BlockRegistry::get().getBlock(str.block_id);
	if (!b.hasBounds())
		return false;

	auto& strr = w->getBlock(x, y + 3);
	auto& bb = BlockRegistry::get().getBlock(strr.block_id);

	return bb.hasBounds();
}

void BlockDoorClose::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("block/door");
}

BlockPainting::BlockPainting()
	: MultiBlock(BLOCK_PAINTING)
{
	m_opacity = 1;
	m_width = 4;
	m_height = 3;
	m_texture_pos = {3, 8};
	setNoCollisionBox();
}

bool BlockPainting::canBePlaced(World* w, int x, int y) const
{
	for (int xx = 0; xx < m_width; ++xx)
	{
		for (int yy = 0; yy < m_height; ++yy)
		{
			auto& str = w->getBlock(x + xx, y + yy);
			if (str.isWallFree() || !str.isAir()) //need wall and not blocks
				return false;
		}
	}
	return true;
}


BlockTree::BlockTree()
	: Block(BLOCK_TREE)
{
	m_opacity = 1;
	setNoCollisionBox();
}

int BlockTree::getCornerOffset(int x, int y, const BlockStruct&) const
{
	return BLOCK_STATE_FULL;
}

bool BlockTree::onNeighbourBlockChange(World* world, int x, int y) const
{
	return false;
}
