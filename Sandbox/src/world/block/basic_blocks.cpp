﻿#include "ndpch.h"
#include "basic_blocks.h"
#include <utility>
#include "world/World.h"
#include "block_datas.h"
#include "world/Camera.h"
#include "graphics/BlockTextureAtlas.h"
#include "world/entity/EntityRegistry.h"
#include "world/particle/particles.h"
#include "world/entity/entities.h"
#include "world/entity/EntityAllocator.h"

//AIR=======================================
BlockAir::BlockAir()
	: Block("air")
{
	setNoCollisionBox();
	m_block_connect_group = BIT(BLOCK_GROUP_AIR_BIT);
	m_opacity = OPACITY_AIR;
	m_texture_pos = -1;
	setFlag(BLOCK_FLAG_HAS_ITEM_VERSION, false);
	setFlag(BLOCK_FLAG_SOLID, false);
	setFlag(BLOCK_FLAG_REPLACEABLE, true);
}

int BlockAir::getTextureOffset(int x, int y, const BlockStruct&) const { return -1; }


bool BlockAir::onNeighborBlockChange(BlockAccess& world, int x, int y) const { return false; }


//STONE=======================================

//PLATFORM====================================
BlockPlatform::BlockPlatform()
	: Block("platform") {}

bool BlockPlatform::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	BlockStruct& block = *world.getBlockM(x, y);
	int lastMeta = block.block_metadata;

	int idLeft = (*world.getBlockM(x - 1, y)).block_id;
	int idRight = (*world.getBlockM(x + 1, y)).block_id;

	bool leftPlat = getID() == idLeft;
	bool rightPlat = getID() == idRight;

	bool isLeftAir = idLeft == BLOCK_AIR;
	bool isRightAir = idRight == BLOCK_AIR;


	if (leftPlat && rightPlat) { block.block_metadata = half_int(1, 0); }
	else if (!rightPlat && !leftPlat && !isLeftAir && !isRightAir) { block.block_metadata = half_int(0, 0); }
	else if (isLeftAir && isRightAir) { block.block_metadata = half_int(6, 0); }
	else if (isLeftAir && rightPlat) { block.block_metadata = half_int(4, 0); }
	else if (isRightAir && leftPlat) { block.block_metadata = half_int(5, 0); }
	else if (!isLeftAir && !leftPlat) { block.block_metadata = half_int(2, 0); }
	else { block.block_metadata = half_int(3, 0); }
	return lastMeta != block.block_metadata; //we have a change (or not)
}

int BlockPlatform::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_pos + (half_int)s.block_metadata; //use corner as block state
}

//GRASS=======================================

BlockGrass::BlockGrass()
	: Block("grass") {}


int BlockGrass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_pos + (half_int)s.block_metadata;
}


bool BlockGrass::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	auto& block = *world.getBlockM(x, y);
	int lastid = block.block_id;
	int lastCorner = block.block_corner;
	//bool custombit = (lastCorner & BIT(4)); //perserve custombit
	//block.block_corner &= ~BIT(4);
	bool custombit = lastCorner & BLOCK_STATE_HAMMER; //perserve custombit
	block.block_corner &= ~BLOCK_STATE_HAMMER;


	Block::onNeighborBlockChange(world, x, y); //update corner state
	if (block.block_corner == BLOCK_STATE_FULL
		|| block.block_corner == BLOCK_STATE_LINE_DOWN
		|| block.block_corner == BLOCK_STATE_LINE_LEFT
		|| block.block_corner == BLOCK_STATE_LINE_RIGHT
		|| block.block_corner == BLOCK_STATE_CORNER_DOWN_LEFT
		|| block.block_corner == BLOCK_STATE_CORNER_DOWN_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_DOWN
		|| block.block_corner == BLOCK_STATE_LINE_VERTICAL)
	{
		block.block_id = BLOCK_DIRT; //no air around
		custombit = false; //we dont care about cracks anymore
	}
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_LEFT
		|| block.block_corner == BLOCK_STATE_LINE_END_LEFT)
		block.block_metadata = half_int(x & 1, 0);
	else if (block.block_corner == BLOCK_STATE_CORNER_UP_RIGHT
		|| block.block_corner == BLOCK_STATE_LINE_END_RIGHT)
		block.block_metadata = half_int((x & 1) + 2, 0);
	else
		block.block_metadata = half_int(x & 3, 1);
	block.block_corner |= BLOCK_STATE_HAMMER * custombit; //put back custombit
	return lastCorner != block.block_corner || lastid != block.block_id; //we have a change (or not)
}

//GLASS=======================================

BlockGlass::BlockGlass()
	: Block("glass") {}

int BlockGlass::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_pos + half_int(s.block_metadata, 0);
}

bool BlockGlass::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	auto c = Block::onNeighborBlockChange(world, x, y);
	auto& e = *world.getBlockM(x, y);
	e.block_metadata = std::rand() % 10;
	e.block_metadata &= 3;
	return c;
}


//up 0 left 1 down 2 right 3
BlockTorch::BlockTorch()
	: Block("torch") {}

void BlockTorch::onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	auto tileE = w.getLoadedTileEntity(x, y);
	if (tileE)
		w.killTileEntity(tileE->getID());
	else
	{
		auto entityBuff = (TileEntityTorch*)EntityAllocator::createEntity(ENTITY_TYPE_TILE_TORCH);
		entityBuff->getPosition() = {x, y};
		w.spawnEntity(entityBuff);
	}
}

int BlockTorch::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	return m_texture_pos + half_int(b.block_corner, 0);
}


bool BlockTorch::isInTorchGroup(BlockAccess& world, int x, int y) const
{
	int id = BLOCK_AIR;
	auto p = world.getBlockM(x, y);
	if (p)
		id = p->block_id;

	return id != BLOCK_TORCH && id != BLOCK_AIR;
}

bool BlockTorch::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	auto& s = *world.getBlockM(x, y);
	int lastCorner = s.block_corner;

	int mask = 0;
	mask |= (isInTorchGroup(world, x - 1, y) & 1) << 0;
	mask |= (isInTorchGroup(world, x, y - 1) & 1) << 1;
	mask |= (isInTorchGroup(world, x + 1, y) & 1) << 2;
	mask |= !s.isWallFree() << 3; //can be placed on wall

	if ((BIT(lastCorner) & mask) == 0)
	{
		if ((mask & BIT(0)) != 0) { s.block_corner = 0; }
		else if ((mask & BIT(1)) != 0) { s.block_corner = 1; }
		else if ((mask & BIT(2)) != 0) { s.block_corner = 2; }
		else if ((mask & BIT(3)) != 0) { s.block_corner = 3; }
		else
		{
			world.setBlock(x, y, BLOCK_AIR); //torch cannot float
		}
	}
	return lastCorner != s.block_corner;
}

BlockDoor::BlockDoor(std::string id)
	: MultiBlock(std::move(id))
{
	m_opacity = OPACITY_AIR;
	m_collision_box = &BLOCK_BOUNDS_DOOR;
	m_collision_box_size = 1;
	setFlag(BLOCK_FLAG_HAS_ITEM_VERSION, true);
}

void BlockDoor::onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& curBlok) const
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
				if (!w.isAir(mainBlockX - 1, mainBlockY + yyy))
					return;
			}
			w.setBlockWithNotify(mainBlockX - 1, mainBlockY, bnew);
		}
		else
		{
			//open to right
			bnew.block_metadata = quarter_int(0, 0, 0, 0);
			for (int yyy = 0; yyy < 3; ++yyy)
			{
				if (!w.isAir(mainBlockX + 1, mainBlockY + yyy))
					return;
			}
			w.setBlockWithNotify(mainBlockX, mainBlockY, bnew);
		}
	}
	else
	{
		BlockStruct bnew = 0;
		bnew.block_id = BLOCK_DOOR_CLOSE;
		bool z = quarter_int(curBlok.block_metadata).z == 1;
		BlockRegistry::get().getBlock(BLOCK_DOOR_OPEN).onBlockDestroyed(w, e, mainBlockX, mainBlockY,
		                                                                *w.getBlockM(mainBlockX, mainBlockY));
		w.setBlockWithNotify(mainBlockX + (z ? 1 : 0), mainBlockY, bnew);
	}
}

std::string BlockDoor::getItemIDFromBlock() const { return "door"; }

BlockDoorOpen::BlockDoorOpen()
	: BlockDoor("door_open")
{
	m_height = 3;
	m_width = 2;
	m_collision_box_size = 0;
}

void BlockDoorOpen::onTextureLoaded(const BlockTextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("block/door") + half_int(1, 0);
}

BlockDoorClose::BlockDoorClose()
	: BlockDoor("door_close")
{
	m_height = 3;
	m_width = 1;
}

bool BlockDoorClose::canBePlaced(World& w, int x, int y) const
{
	if (!MultiBlock::canBePlaced(w, x, y))
		return false;
	auto& str = *w.getBlockM(x, y - 1);
	auto& b = BlockRegistry::get().getBlock(str.block_id);
	if (!b.hasCollisionBox())
		return false;

	auto& strr = *w.getBlockM(x, y + 3);
	auto& bb = BlockRegistry::get().getBlock(strr.block_id);

	return bb.hasCollisionBox();
}

void BlockDoorClose::onTextureLoaded(const BlockTextureAtlas& atlas) { m_texture_pos = atlas.getTexture("block/door"); }

BlockPumpkin::BlockPumpkin()
	: MultiBlock("pumpkin") {}

void BlockPumpkin::onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	MultiBlock::onBlockPlaced(w, e, x, y, b);

	for (int xx = 0; xx < m_width; ++xx)
	{
		for (int yy = -3; yy < 0; ++yy)
		{
			auto& str = *w.getBlockM(x + xx, y + yy);
			if (str.block_id != BLOCK_SNOW) //need wall and not blocks
				return;
		}
	}


	for (int xx = 0; xx < m_width; ++xx)
		for (int yy = -3; yy < m_height; ++yy)
		{
			w.setBlockWithNotify(x + xx, y + yy, 0);
			w.spawnParticle(ParticleList::dot, glm::vec2(x + xx + 0.5f, y + yy + 0.5f), glm::vec2(0), glm::vec2(0), -1);
		}
	//we have snowman!
	auto man = EntityAllocator::createEntity(ENTITY_TYPE_SNOWMAN);
	man->getPosition() = {x + m_width / 2, y};
	w.spawnEntity(man);
}

BlockChest::BlockChest()
	: MultiBlock("chest") {}

void BlockChest::onTextureLoaded(const BlockTextureAtlas& atlas)
{
	m_open_texture = atlas.getTexture("block/" + getStringID() + "_opened");
	m_close_texture = atlas.getTexture("block/" + getStringID() + "_closed");
}


int BlockChest::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	quarter_int d = b.block_metadata;

	return (d.z == 0 ? m_close_texture : m_open_texture) + half_int(d.x, d.y);
}

void BlockChest::openChest(World& w, int x, int y, bool open) const
{
	auto basePos = getTileEntityCoords(x, y, *w.getBlock(x, y));
	for (int x = 0; x < m_width; ++x)
		for (int y = 0; y < m_height; ++y)
			((quarter_int*)&w.getBlockM(basePos.x + x, basePos.y + y)->block_metadata)->z = open ? 1 : 0;
	w.getChunkM(Chunk::getChunkIDFromWorldPos(x, y))->markDirty(true);
}

bool BlockChest::isOpened(BlockStruct& b) const { return ((quarter_int*)&b.block_metadata)->z == 1; }


BlockTreeSapling::BlockTreeSapling()
	: Block("tree_sapling") {}

bool BlockTreeSapling::canBePlaced(World& w, int x, int y) const
{
	if (!w.isAir(x, y))
		return false;
	int underID = w.getBlockM(x, y - 1)->block_id;
	return underID == BLOCK_DIRT || underID == BLOCK_GRASS;
}


BlockTree::BlockTree()
	: Block("tree") {}

void BlockTree::onTextureLoaded(const BlockTextureAtlas& atlas)
{
	using namespace BlockTreeScope;
	std::string path = "block/tree";

	m_texture_map[RootL] = atlas.getTexture(path, "RootL");
	m_texture_map[RootR] = atlas.getTexture(path, "RootR");
	m_texture_map[fullTrunk2W] = atlas.getTexture(path, "fullTrunk2W");

	m_texture_map[dryBranchL] = atlas.getTexture(path, "dryBranchL");
	m_texture_map[dryBranchR] = atlas.getTexture(path, "dryBranchR");

	m_texture_map[branchL] = atlas.getTexture(path, "branchL");
	m_texture_map[branchR] = atlas.getTexture(path, "branchR");

	m_texture_map[trunkBranchL] = atlas.getTexture(path, "trunkBranchL");
	m_texture_map[trunkBranchR] = atlas.getTexture(path, "trunkBranchR");

	m_texture_map[smallBlobL2W] = atlas.getTexture(path, "smallBlobL2W");
	m_texture_map[smallBlobR2W] = atlas.getTexture(path, "smallBlobR2W");

	m_texture_map[thinTrunkL] = atlas.getTexture(path, "thinTrunkL");
	m_texture_map[thinTrunkR] = atlas.getTexture(path, "thinTrunkR");

	m_texture_map[dryBlobL] = atlas.getTexture(path, "dryBlobL");
	m_texture_map[dryBlobR] = atlas.getTexture(path, "dryBlobR");

	m_texture_map[bigBlob0] = atlas.getTexture(path, "bigBlob0");
	m_texture_map[bigBlob1] = atlas.getTexture(path, "bigBlob1");
	m_texture_map[blob] = atlas.getTexture(path, "blob");

	m_texture_map[trunkL] = atlas.getTexture(path, "trunkL");
	m_texture_map[trunkR] = atlas.getTexture(path, "trunkR");
}

int BlockTree::getTextureOffset(int x, int y, const BlockStruct& s) const
{
	return m_texture_map[(uint8_t)s.block_corner] + (half_int)s.block_metadata;
}


bool BlockTree::onNeighborBlockChange(BlockAccess& world, int x, int y) const { return false; }

void BlockTree::onBlockDestroyed(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	auto& left = *w.getBlockM(x - 1, y);
	auto& right = *w.getBlockM(x + 1, y);
	auto& up = *w.getBlockM(x, y + 1);
	auto& down = *w.getBlockM(x, y - 1);

	half_int baseBlock = half_int(x, y) - half_int(b.block_metadata);
	b.block_id = 0;
	//we dont want other blocks to call this block which would call those blocks which would c.... you get the idea

	bool isOurL = left.block_id == getID(); //&&
	//half_int(half_int(x - 1, y) - half_int(left.block_metadata)) == baseBlock;
	if (isOurL)
		w.setBlockWithNotify(x - 1, y, 0);

	bool isOurR = right.block_id == getID(); //&&
	//	half_int(half_int(x + 1, y) - half_int(right.block_metadata)) == baseBlock;
	if (isOurR)
		w.setBlockWithNotify(x + 1, y, 0);

	bool isOurU = up.block_id == getID(); //&&
	//	half_int(half_int(x, y + 1) - half_int(up.block_metadata)) == baseBlock;
	if (isOurU)
		w.setBlockWithNotify(x, y + 1, 0);

	bool isOurD = down.block_id == getID(); // &&
	//	half_int(half_int(x, y - 1) - half_int(down.block_metadata)) == baseBlock;
	if (isOurD)
		w.setBlockWithNotify(x, y - 1, 0);
}

BlockPlant::BlockPlant(std::string id)
	: Block(std::move(id))

{
	setNoCollisionBox();
	setFlag(BLOCK_FLAG_CANNOT_FLOAT, true);
	setFlag(BLOCK_FLAG_SOLID, false);
	m_opacity = OPACITY_AIR;
}

int BlockPlant::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	return m_texture_pos + half_int(half_int(b.block_metadata).x, 0);
}

bool BlockPlant::canBePlaced(World& w, int x, int y) const
{
	if (!w.isAir(x, y))
		return false;
	int underID = w.getBlockM(x, y - 1)->block_id;
	return underID == BLOCK_GRASS;
}

void BlockPlant::onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	b.block_metadata = std::rand() % m_max_metadata;
}

bool BlockPlant::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	if (world.getBlockM(x, y - 1)->block_id != BLOCK_GRASS)
	{
		world.setBlockWithNotify(x, y, 0);
		return true;
	}
	return false;
}
