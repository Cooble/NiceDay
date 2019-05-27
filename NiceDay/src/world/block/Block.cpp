#include "ndpch.h"
#include "world/World.h"
#include "Block.h"


Block::Block(int id)
	:m_id(id),
	m_texture_pos(0),
	m_corner_translate_array(nullptr),
	m_has_big_texture(false),
	m_opacity(0.25f),
	m_block_connect_group(0)
{
}

Block::~Block() = default;

float Block::getOpacity(const BlockStruct&) const
{
	return m_opacity;
}

int Block::getTextureOffset(int x, int y, const BlockStruct&)const
{
	if (!m_has_big_texture)
		return m_texture_pos.i;

	return half_int(m_texture_pos.x * 4 + (x & 3), m_texture_pos.y * 4 + (y & 3)).i;
}

int Block::getCornerOffset(int x, int y, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	return m_corner_translate_array[(int)b.block_corner].i;
}

bool Block::isInGroup(World* w, int x, int y, int group) const
{
	const BlockStruct* b = w->getBlockPointer(x, y);
	if (b)
	{
		auto& bl = BlockRegistry::get().getBlock(b->block_id);
		return bl.isInConnectGroup(group)|| bl.getID()==m_id;
	}
	return false;
}

bool Block::onNeighbourBlockChange(World* world, int x, int y) const
{

	bool up = !isInGroup(world, x, y + 1, m_block_connect_group);
	bool down = !isInGroup(world, x, y - 1, m_block_connect_group);
	bool left = !isInGroup(world, x - 1, y, m_block_connect_group);
	bool right = !isInGroup(world, x + 1, y, m_block_connect_group);

	BlockStruct& block = world->editBlock(x, y);
	int lastCorner = block.block_corner;
	//bit
	if (up && down && left && right)//todo optimalize comparisons using bitwise operators
	{
		block.block_corner = BLOCK_STATE_BIT;
	}
	else if (!up && !down && !left && !right)
	{
		block.block_corner = BLOCK_STATE_FULL;
	}
	//thin lines
	else if (up && down && !left && !right)
	{
		block.block_corner = BLOCK_STATE_LINE_HORIZONTAL;
	}
	else if (!up && !down && left && right)
	{
		block.block_corner = BLOCK_STATE_LINE_VERTICAL;
	}
	//bold lines
	else if (up && !down && !left && !right)
	{
		block.block_corner = BLOCK_STATE_LINE_UP;
	}
	else if (!up && down && !left && !right)
	{
		block.block_corner = BLOCK_STATE_LINE_DOWN;
	}
	else if (!up && !down && left && !right)
	{
		block.block_corner = BLOCK_STATE_LINE_LEFT;
	}
	else if (!up && !down && !left && right)
	{
		block.block_corner = BLOCK_STATE_LINE_RIGHT;
	}
	//ends
	else if (up && down && left && !right)
	{
		block.block_corner = BLOCK_STATE_LINE_END_LEFT;
	}
	else if (up && down && !left && right)
	{
		block.block_corner = BLOCK_STATE_LINE_END_RIGHT;
	}
	else if (up && !down && left && right)
	{
		block.block_corner = BLOCK_STATE_LINE_END_UP;
	}
	else if (!up && down && left && right)
	{
		block.block_corner = BLOCK_STATE_LINE_END_DOWN;
	}
	//corners
	else if (up && !down && left && !right)
	{
		block.block_corner = BLOCK_STATE_CORNER_UP_LEFT;
	}
	else if (up && !down && !left && right)
	{
		block.block_corner = BLOCK_STATE_CORNER_UP_RIGHT;
	}
	else if (!up && down && left && !right)
	{
		block.block_corner = BLOCK_STATE_CORNER_DOWN_LEFT;
	}
	else if (!up && down && !left && right)
	{
		block.block_corner = BLOCK_STATE_CORNER_DOWN_RIGHT;
	}

	return lastCorner != block.block_corner;//we have a change (or not)

}

Wall::Wall(int id)
	:m_id(id),
	m_texture_pos(0),
	m_corner_translate_array(nullptr),
	m_opaque(false)
{
}

Wall::~Wall() = default;

bool Wall::isOpaque(const BlockStruct&) const { return m_opaque; }

int Wall::getTextureOffset(int wx, int wy, const BlockStruct&) const
{
	return half_int(m_texture_pos.x * 8 + (wx & 7), m_texture_pos.y * 8 + (wy & 7)).i;
}

int Wall::getCornerOffset(int wx, int wy, const BlockStruct& b) const
{
	ASSERT(m_corner_translate_array, " m_corner_translate_array  cannot be null");
	
	half_int i = m_corner_translate_array[(int)b.wall_corner[(wy & 1) * 2 + (wx & 1)]];
	if (i.i == 0)
		return 0;

	return half_int( (((wx & 1)+ (wy & 1))&1)*3,0).plus(i).i;
}
static bool isWallFree(World * w, int x, int y)
{
	auto b = w->getBlockPointer(x, y);
	if (b)
		return b->isWallFree();
	return false;//if we dont know we will assume is occupied
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
	b = w->getBlockPointer(x - 1, y + 1);
	if (b)
	{
		if (isWallFree(w, x - 1, y + 1) 
			&& w->getBlock(x - 1, y).isWallFree() 
			&& w->getBlock(x, y + 1).isWallFree()) 
		{
			BlockStruct& b = w->editBlock(x - 1, y + 1);
			b.wall_id[1] = m_id;
			b.wall_corner[1] = BLOCK_STATE_CORNER_UP_LEFT;
		}
	}
	//corner left down
	b = w->getBlockPointer(x - 1, y - 1);
	if (b)
	{
		if (isWallFree(w, x - 1, y - 1)
			&& w->getBlock(x - 1, y).isWallFree()
			&& w->getBlock(x, y - 1).isWallFree())
		{
			BlockStruct& b = w->editBlock(x - 1, y - 1);
			b.wall_id[3] = m_id;
			b.wall_corner[3] = BLOCK_STATE_CORNER_DOWN_LEFT;
		}
	}
	//corner right up
	b = w->getBlockPointer(x + 1, y + 1);
	if (b)
	{
		if (isWallFree(w, x + 1, y + 1)
			&& w->getBlock(x + 1, y).isWallFree()
			&& w->getBlock(x, y + 1).isWallFree())
		{
			BlockStruct& b = w->editBlock(x + 1, y + 1);
			b.wall_id[0] = m_id;
			b.wall_corner[0] = BLOCK_STATE_CORNER_UP_RIGHT;
		}
	}
	//corner right down
	b = w->getBlockPointer(x + 1, y - 1);
	if (b)
	{
		if (isWallFree(w, x + 1, y - 1)
			&& w->getBlock(x + 1, y).isWallFree()
			&& w->getBlock(x, y - 1).isWallFree())
		{
			BlockStruct& b = w->editBlock(x + 1, y - 1);
			b.wall_id[2] = m_id;
			b.wall_corner[2] = BLOCK_STATE_CORNER_DOWN_RIGHT;
		}
	}
}


