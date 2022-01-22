#include "ndpch.h"

#include "world/World.h"
#include "Block.h"
#include "block_datas.h"
#include "graphics/BlockTextureAtlas.h"
#include "world/entity/EntityRegistry.h"
#include "world/entity/EntityAllocator.h"
#include "inventory/Item.h"
#include "inventory/ItemBlock.h"
#include "inventory/ItemRegistry.h"
#include "inventory/ItemStack.h"

using namespace nd;

Block::Block(std::string id)
	: m_id(-1),
	  m_string_id(std::move(id)),
	  m_texture_pos(0),
	  m_corner_translate_array(nullptr),
	  m_tile_entity(ENTITY_TYPE_NONE),
	  m_collision_box(BLOCK_BOUNDS_DEFAULT),
	  m_collision_box_size(3),
	  m_opacity(OPACITY_SOLID),
	  m_light_src(0),
	  m_block_connect_group(0)
{
	setFlag(BLOCK_FLAG_HAS_ITEM_VERSION, true);
	setFlag(BLOCK_FLAG_SOLID, true);
}

Phys::Vecti Block::getTileEntityCoords(int x, int y, const BlockStruct& b) const { return Phys::Vecti(x, y); }

const Phys::Polygon& Block::getCollisionBox(int x, int y, const BlockStruct& b) const
{
	auto i = m_collision_box_size >= 3 ? 1 : 0;
	switch (BLOCK_STATE_PURE_MASK & b.block_corner)
	{
	case BLOCK_STATE_CORNER_UP_LEFT:
		return m_collision_box[1 * i];
	case BLOCK_STATE_CORNER_UP_RIGHT:
		return m_collision_box[2 * i];
	case BLOCK_STATE_CORNER_DOWN_LEFT:
		return m_collision_box[3 * i];
	case BLOCK_STATE_CORNER_DOWN_RIGHT:
		return m_collision_box[4 * i];
	case BLOCK_STATE_LINE_UP:
		return m_collision_box[5 * i * (bool)(b.block_corner&BLOCK_STATE_HAMMER)];//needs to be hammer to work
	case BLOCK_STATE_LINE_DOWN:
		return m_collision_box[6 * i * (bool)(b.block_corner&BLOCK_STATE_HAMMER)];//needs to be hammer to work
	default:
		return m_collision_box[0];
	}
}

bool Block::hasCollisionBox() const { return m_collision_box_size != 0; }

void Block::onTextureLoaded(const BlockTextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("block/" + getStringID());
}

int Block::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	if (!hasBigTexture())
		return m_texture_pos.i;

	return half_int(m_texture_pos.x + (x & 3), m_texture_pos.y + (y & 3)).i;
}

int Block::getCornerOffset(int x, int y, const BlockStruct& b) const
{
	auto index = (int)b.block_corner;
	return m_corner_translate_array ? m_corner_translate_array[index].i : BLOCK_STATE_FULL;
}

bool Block::isInGroup(BlockAccess& w, int x, int y, int group) const
{
	auto b = w.getBlockM(x, y);
	if (b)
	{
		auto& bl = BlockRegistry::get().getBlock(b->block_id);
		return bl.isInConnectGroup(group) || bl.getID() == m_id;
	}
	return false;
}

bool Block::isInGroup(BlockID blockID, int group) const
{
	auto& bl = BlockRegistry::get().getBlock(blockID);
	return bl.isInConnectGroup(group) || bl.getID() == m_id;
}

bool Block::onNeighborBlockChange(BlockAccess& world, int x, int y) const
{
	int mask = 0;
	mask |= (!isInGroup(world, x, y + 1, m_block_connect_group) & 1) << 0;
	mask |= (!isInGroup(world, x - 1, y, m_block_connect_group) & 1) << 1;
	mask |= (!isInGroup(world, x, y - 1, m_block_connect_group) & 1) << 2;
	mask |= (!isInGroup(world, x + 1, y, m_block_connect_group) & 1) << 3;


	BlockStruct& block = *world.getBlockM(x, y);
	int lastCorner = block.block_corner;
	block.block_corner = mask | (lastCorner & ~BLOCK_STATE_PURE_MASK);
	return lastCorner != block.block_corner; //we have a change (or not)
}

void Block::onBlockPlaced(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	if (hasTileEntity())
	{
		auto entity = EntityAllocator::createEntity(m_tile_entity);
		entity->getPosition() = {x, y};
		w.spawnEntity(entity);
	}
}

void Block::onBlockDestroyed(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	if (hasTileEntity())
	{
		auto corrds = getTileEntityCoords(x, y, b);
		WorldEntity* ee = w.getLoadedTileEntity(corrds.x, corrds.y);
		if (ee)
			ee->markDead();
	}
}

void Block::onBlockClicked(World& w, WorldEntity* e, int x, int y, BlockStruct& b) const
{
	if (hasTileEntity())
	{
		auto pos = getTileEntityCoords(x, y, b);
		auto entity = dynamic_cast<TileEntity*>(w.getLoadedTileEntity(pos.x, pos.y));
		if (entity == nullptr)
		{
			entity = (TileEntity*)EntityAllocator::createEntity(m_tile_entity);
			entity->getPosition() = {pos.x, pos.y};
			w.spawnEntity(entity);
		}
		entity->onClicked(w, e);
	}
}

bool Block::canBePlaced(World& w, int x, int y) const
{
	// can be hanged only on wall
	if (needsWall() && w.getBlock(x, y)->isWallFree())
		return false;

	// block is already occupied with another block
	if (!w.getBlockInstance(x, y).isReplaceable())
		return false;

	// can be put only on ground
	if (cannotFloat() && !w.getBlockInstance(x, y - 1).isSolid())
		return false;

	return true;
}

const ItemBlock& Block::getItemFromBlock() const
{
	return dynamic_cast<const ItemBlock&>(ItemRegistry::get().getItem(SID(getItemIDFromBlock())));
}

std::string Block::getItemIDFromBlock() const { return m_string_id; }

ItemStack* Block::createItemStackFromBlock(const BlockStruct& b) const
{
	if (!hasItemVersion())
		return nullptr;
	auto out = ItemStack::create(SID(getItemIDFromBlock()), 1);
	if (m_max_metadata != 0)
		out->setMetadata(b.block_metadata);
	return out;
}


//MULTIBLOCK==================================================
MultiBlock::MultiBlock(std::string id)
	: Block(std::move(id)) {}

Phys::Vecti MultiBlock::getTileEntityCoords(int x, int y, const BlockStruct& b) const
{
	quarter_int offset = b.block_metadata;
	return {x - offset.x, y - offset.y};
}

int MultiBlock::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	quarter_int d = b.block_metadata;

	return m_texture_pos + half_int(d.x, d.y);
}

bool MultiBlock::onNeighborBlockChange(BlockAccess& world, int x, int y) const { return false; }

void MultiBlock::onBlockPlaced(World& w, WorldEntity* e, int xx, int yy, BlockStruct& b) const
{
	quarter_int meta = b.block_metadata;
	if (!(meta.x == 0 && meta.y == 0)) //we dont want to call onblockplaced on segments placed by this method
		return;
	for (int x = 0; x < m_width; ++x)
	{
		for (int y = 0; y < m_height; ++y)
		{
			if (x == 0 && y == 0)
				continue; //skip ourselves
			BlockStruct segment;
			segment.block_id = getID();
			quarter_int offset(x, y, meta.z, meta.w);
			segment.block_metadata = offset;
			w.setBlockWithNotify(xx + x, yy + y, segment);
		}
	}
	Block::onBlockPlaced(w, e, xx, yy, b);
}

void MultiBlock::onBlockDestroyed(World& w, WorldEntity* e, int xx, int yy, BlockStruct& b) const
{
	quarter_int offset = b.block_metadata;
	quarter_int clearValue(127, 127, 0, 0);

	if (offset == clearValue)
		return;

	for (int x = 0; x < m_width; ++x)
	{
		for (int y = 0; y < m_height; ++y)
		{
			//make this block skip this function when destroyed
			w.getBlockM(xx + x - offset.x, yy + y - offset.y)->block_metadata = clearValue;
			w.setBlockWithNotify(xx + x - offset.x, yy + y - offset.y, 0);
		}
	}
	Block::onBlockDestroyed(w, e, xx - offset.x, yy - offset.y, *w.getBlockM(xx - offset.x, yy - offset.y));
}

bool MultiBlock::canBePlaced(World& w, int xx, int yy) const
{
	for (int x = 0; x < m_width; ++x)
		for (int y = 0; y < m_height; ++y)
		{
			auto b = w.getBlock(xx + x, yy + y);
			if (!b->isAir() || needsWall() && b->isWallFree())
				return false;
		}
	if (cannotFloat())
		for (int x = 0; x < m_width; ++x)
		{
			if (!w.getBlockInstance(xx + x, yy - 1).isSolid())
				return false;
		}
	return true;
}


//WALL======================================================
Wall::Wall(std::string id)
	: m_string_id(std::move(id)),
	  m_texture_pos(0),
	  m_corner_translate_array(BlockRegistry::get().getCorners("dirt", true))
{
	setFlag(BLOCK_FLAG_HAS_ITEM_VERSION, true);
	setFlag(BLOCK_FLAG_SOLID, true);
}

Wall::~Wall() = default;


void Wall::onTextureLoaded(const BlockTextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("wall/" + getStringID());
}

int Wall::getTextureOffset(int wx, int wy, const BlockStruct&) const
{
	return half_int(m_texture_pos.x * 2 + (wx & 7), m_texture_pos.y * 2 + (wy & 7)).i;
}

int Wall::getCornerOffset(int wx, int wy, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	half_int i = m_corner_translate_array[(int)b.wall_corner[(wy & 1) * 2 + (wx & 1)]];
	if (i.i == 0)
		return 0;

	return half_int(((wx & 1) + (wy & 1) & 1) * 3, 0).plus(i).i;
}

static bool isWallFree(BlockAccess& w, int x, int y)
{
	auto b = w.getBlockM(x, y);
	return b ? b->isWallFree() : false; //if we dont know we will assume is occupied
}

void Wall::onNeighbourWallChange(BlockAccess& w, int x, int y) const
{
	BlockStruct& block = *w.getBlockM(x, y);
	auto p = w.getBlockM(x, y);
	ASSERT(p, "fuk");
	//left
	if (isWallFree(w, x - 1, y))
	{
		auto& b = *w.getBlockM(x - 1, y);
		b.wall_id[1] = m_id;
		b.wall_id[3] = m_id;
		b.wall_corner[1] = BLOCK_STATE_LINE_LEFT;
		b.wall_corner[3] = BLOCK_STATE_LINE_LEFT;
	}
	//right
	if (isWallFree(w, x + 1, y))
	{
		auto& b = *w.getBlockM(x + 1, y);
		b.wall_id[0] = m_id;
		b.wall_id[2] = m_id;
		b.wall_corner[0] = BLOCK_STATE_LINE_RIGHT;
		b.wall_corner[2] = BLOCK_STATE_LINE_RIGHT;
	}
	//up
	if (isWallFree(w, x, y + 1))
	{
		auto& b = *w.getBlockM(x, y + 1);
		b.wall_id[0] = m_id;
		b.wall_id[1] = m_id;
		b.wall_corner[0] = BLOCK_STATE_LINE_UP;
		b.wall_corner[1] = BLOCK_STATE_LINE_UP;
	}
	//down
	if (isWallFree(w, x, y - 1))
	{
		auto& b = *w.getBlockM(x, y - 1);
		b.wall_id[2] = m_id;
		b.wall_id[3] = m_id;
		b.wall_corner[2] = BLOCK_STATE_LINE_DOWN;
		b.wall_corner[3] = BLOCK_STATE_LINE_DOWN;
	}
	//corner left up

	if (isWallFree(w, x - 1, y + 1)
		&& isWallFree(w, x - 1, y)
		&& isWallFree(w, x, y + 1))
	{
		auto& b = *w.getBlockM(x - 1, y + 1);
		b.wall_id[1] = m_id;
		b.wall_corner[1] = BLOCK_STATE_CORNER_UP_LEFT;
	}

	//corner left down

	if (isWallFree(w, x - 1, y - 1)
		&& isWallFree(w, x - 1, y)
		&& isWallFree(w, x, y - 1))
	{
		auto& b = *w.getBlockM(x - 1, y - 1);
		b.wall_id[3] = m_id;
		b.wall_corner[3] = BLOCK_STATE_CORNER_DOWN_LEFT;
	}

	//corner right up

	if (isWallFree(w, x + 1, y + 1)
		&& isWallFree(w, x + 1, y)
		&& isWallFree(w, x, y + 1))
	{
		auto& b = *w.getBlockM(x + 1, y + 1);
		b.wall_id[0] = m_id;
		b.wall_corner[0] = BLOCK_STATE_CORNER_UP_RIGHT;
	}

	//corner right down

	if (isWallFree(w, x + 1, y - 1)
		&& isWallFree(w, x + 1, y)
		&& isWallFree(w, x, y - 1))
	{
		auto& b = *w.getBlockM(x + 1, y - 1);
		b.wall_id[2] = m_id;
		b.wall_corner[2] = BLOCK_STATE_CORNER_DOWN_RIGHT;
	}
}

bool Wall::canBePlaced(World& w, int x, int y) const
{
	auto block = w.getBlockM(x, y);
	return block->isWallFree() || BlockRegistry::get().getWall(block->wallID()).isReplaceable();
}

const ItemWall& Wall::getItemFromWall() const
{
	return dynamic_cast<const ItemWall&>(ItemRegistry::get().getItem(SID(getItemIDFromWall())));
}

ItemStack* Wall::createItemStackFromWall(const BlockStruct& b) const
{
	if (!hasItemVersion())
		return nullptr;
	auto out = ItemStack::create(SID(getItemIDFromWall()), 1);
	//if (m_max_metadata != 0)
	//	out->setMetadata(b.block_metadata);
	return out;
}

std::string Wall::getItemIDFromWall() const { return m_string_id + "_wall"; }
