#include "ndpch.h"

#include "world/World.h"
#include "Block.h"
#include "block_datas.h"
#include "graphics/TextureAtlas.h"


Block::Block(int id)
	: m_id(id),
	  m_texture_pos(0),
	  m_corner_translate_array(nullptr),
	  m_has_big_texture(false),
	  m_collision_box(BLOCK_BOUNDS_DEFAULT),
	  m_collision_box_size(1),
	  m_opacity(3),
	  m_light_src(0),
	  m_block_connect_group(0)
{
}

void Block::setNoCollisionBox()
{
	m_collision_box = nullptr;
	m_collision_box_size = 0;
}


const Phys::Polygon& Block::getCollisionBox(int x, int y, const BlockStruct& b) const
{
	return m_collision_box[0];
}

bool Block::hasCollisionBox() const { return m_collision_box_size != 0; }

void Block::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("block/" + toString());
}

int Block::getTextureOffset(int x, int y, const BlockStruct&) const
{
	if (!m_has_big_texture)
		return m_texture_pos.i;

	return half_int(m_texture_pos.x + (x & 3), m_texture_pos.y + (y & 3)).i;
}

int Block::getCornerOffset(int x, int y, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	return m_corner_translate_array[(int)b.block_corner].i;
}

bool Block::isInGroup(World* w, int x, int y, int group) const
{
	const BlockStruct* b = w->getLoadedBlockPointer(x, y);
	if (b)
	{
		auto& bl = BlockRegistry::get().getBlock(b->block_id);
		return bl.isInConnectGroup(group) || bl.getID() == m_id;
	}
	return false;
}

bool Block::isInGroup(int blockID, int group) const
{
	auto& bl = BlockRegistry::get().getBlock(blockID);
	return bl.isInConnectGroup(group) || bl.getID() == m_id;
}

bool Block::onNeighbourBlockChange(World* world, int x, int y) const
{
	int mask = 0;
	mask |= ((!isInGroup(world, x, y + 1, m_block_connect_group)) & 1) << 0;
	mask |= ((!isInGroup(world, x - 1, y, m_block_connect_group)) & 1) << 1;
	mask |= ((!isInGroup(world, x, y - 1, m_block_connect_group)) & 1) << 2;
	mask |= ((!isInGroup(world, x + 1, y, m_block_connect_group)) & 1) << 3;


	BlockStruct& block = world->editBlock(x, y);
	int lastCorner = block.block_corner;
	block.block_corner = mask;
	return lastCorner != block.block_corner; //we have a change (or not)
}


//MULTIBLOCK==================================================
MultiBlock::MultiBlock(int id)
	: Block(id)
{
}

int MultiBlock::getCornerOffset(int x, int y, const BlockStruct& b) const
{
	return BLOCK_STATE_FULL;
}

int MultiBlock::getTextureOffset(int x, int y, const BlockStruct& b) const
{
	quarter_int d = b.block_metadata;

	return m_texture_pos + half_int(d.x, d.y);
}

bool MultiBlock::onNeighbourBlockChange(World* world, int x, int y) const
{
	return false;
}

void MultiBlock::onBlockPlaced(World* w, WorldEntity* e, int xx, int yy, BlockStruct& b) const
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
			w->setBlock(xx + x, yy + y, segment);
		}
	}
}

void MultiBlock::onBlockDestroyed(World* w, WorldEntity* e, int xx, int yy, BlockStruct& b) const
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
			w->editBlock(xx + x - offset.x, yy + y - offset.y).block_metadata = clearValue;
			w->setBlock(xx + x - offset.x, yy + y - offset.y, 0);
		}
	}
}

bool MultiBlock::canBePlaced(World* w, int xx, int yy) const
{
	for (int x = 0; x < m_width; ++x)
		for (int y = 0; y < m_height; ++y)
			if (!w->getBlock(xx + x, yy + y).isAir())
				return false;
	return true;
}


//WALL======================================================
Wall::Wall(int id)
	: m_id(id),
	  m_texture_pos(0),
	  m_corner_translate_array(nullptr),
	  m_transparent(false)
{
}

Wall::~Wall() = default;


void Wall::onTextureLoaded(const TextureAtlas& atlas)
{
	m_texture_pos = atlas.getTexture("wall/" + toString());
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

	return half_int((((wx & 1) + (wy & 1)) & 1) * 3, 0).plus(i).i;
}

static bool isWallFree(World* w, int x, int y)
{
	auto b = w->getLoadedBlockPointer(x, y);
	return b ? b->isWallFree() : false; //if we dont know we will assume is occupied
}

void Wall::onNeighbourWallChange(World* w, int x, int y) const
{
	BlockStruct& block = w->editBlock(x, y);
	//left
	if (isWallFree(w, x - 1, y))
	{
		BlockStruct& b = w->editBlock(x - 1, y);
		b.wall_id[1] = m_id;
		b.wall_id[3] = m_id;
		b.wall_corner[1] = BLOCK_STATE_LINE_LEFT;
		b.wall_corner[3] = BLOCK_STATE_LINE_LEFT;
	}
	//right
	if (isWallFree(w, x + 1, y))
	{
		BlockStruct& b = w->editBlock(x + 1, y);
		b.wall_id[0] = m_id;
		b.wall_id[2] = m_id;
		b.wall_corner[0] = BLOCK_STATE_LINE_RIGHT;
		b.wall_corner[2] = BLOCK_STATE_LINE_RIGHT;
	}
	//up
	if (isWallFree(w, x, y + 1))
	{
		BlockStruct& b = w->editBlock(x, y + 1);
		b.wall_id[0] = m_id;
		b.wall_id[1] = m_id;
		b.wall_corner[0] = BLOCK_STATE_LINE_UP;
		b.wall_corner[1] = BLOCK_STATE_LINE_UP;
	}
	//down
	if (isWallFree(w, x, y - 1))
	{
		BlockStruct& b = w->editBlock(x, y - 1);
		b.wall_id[2] = m_id;
		b.wall_id[3] = m_id;
		b.wall_corner[2] = BLOCK_STATE_LINE_DOWN;
		b.wall_corner[3] = BLOCK_STATE_LINE_DOWN;
	}
	const BlockStruct* b = nullptr;

	//corner left up

	if (isWallFree(w, x - 1, y + 1)
		&& isWallFree(w, x - 1, y)
		&& isWallFree(w, x, y + 1))
	{
		BlockStruct& b = w->editBlock(x - 1, y + 1);
		b.wall_id[1] = m_id;
		b.wall_corner[1] = BLOCK_STATE_CORNER_UP_LEFT;
	}

	//corner left down

	if (isWallFree(w, x - 1, y - 1)
		&& isWallFree(w, x - 1, y)
		&& isWallFree(w, x, y - 1))
	{
		BlockStruct& b = w->editBlock(x - 1, y - 1);
		b.wall_id[3] = m_id;
		b.wall_corner[3] = BLOCK_STATE_CORNER_DOWN_LEFT;
	}

	//corner right up

	if (isWallFree(w, x + 1, y + 1)
		&& isWallFree(w, x + 1, y)
		&& isWallFree(w, x, y + 1))
	{
		BlockStruct& b = w->editBlock(x + 1, y + 1);
		b.wall_id[0] = m_id;
		b.wall_corner[0] = BLOCK_STATE_CORNER_UP_RIGHT;
	}

	//corner right down

	if (isWallFree(w, x + 1, y - 1)
		&& isWallFree(w, x + 1, y)
		&& isWallFree(w, x, y - 1))
	{
		BlockStruct& b = w->editBlock(x + 1, y - 1);
		b.wall_id[2] = m_id;
		b.wall_corner[2] = BLOCK_STATE_CORNER_DOWN_RIGHT;
	}
}
