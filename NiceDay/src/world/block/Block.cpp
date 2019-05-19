#include "ndpch.h"
#include "world/World.h"
#include "Block.h"


Block::Block(int id)
	:m_id(id),
	m_texture_pos(0),
	m_has_big_texture(false),
	m_opacity(0.25f),
	m_block_connect_group(BIT(BLOCK_GROUP_DIRT_BIT))
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
	//return half_int(m_texture_pos.x*4, m_texture_pos.y*4).i;	
}

int Block::getCornerOffset(int x, int y, const BlockStruct& b) const
{
	return b.corner;
}

bool Block::isInGroup(World* w, int x, int y, int group)
{
	const BlockStruct* b = w->getBlockPointer(x, y);
	if (b == nullptr)
		return false;
	return BlockRegistry::get().getBlock(b->id).isInConnectGroup(group);
}

bool Block::onNeighbourBlockChange(World* world, int x, int y) const
{

	bool up =	!isInGroup(world, x, y + 1, m_block_connect_group);
	bool down = !isInGroup(world, x, y - 1, m_block_connect_group);
	bool left = !isInGroup(world, x - 1, y, m_block_connect_group);
	bool right =!isInGroup(world, x + 1, y, m_block_connect_group);

	BlockStruct& block = world->editBlock(x, y);
	int lastCorner = block.corner;
	//bit
	if (up && down && left && right)
	{
		block.corner = BLOCK_STATE_BIT;
	}
	else if (!up && !down && !left && !right)
	{
		block.corner = BLOCK_STATE_FULL;
	}
	//thin lines
	else if (up && down && !left && !right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_HORIZONTAL;
	}
	else if (!up && !down && left && right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_VERTICAL;
	}
	//bold lines
	else if (up && !down && !left && !right)
	{
		block.corner = BLOCK_STATE_BOLD_LINE_UP;
	}
	else if (!up && down && !left && !right)
	{
		block.corner = BLOCK_STATE_BOLD_LINE_DOWN;
	}
	else if (!up && !down && left && !right)
	{
		block.corner = BLOCK_STATE_BOLD_LINE_LEFT;
	}
	else if (!up && !down && !left && right)
	{
		block.corner = BLOCK_STATE_BOLD_LINE_RIGHT;
	}
	//ends
	else if (up && down && left && !right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_END_LEFT;
	}
	else if (up && down && !left && right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_END_RIGHT;
	}
	else if (up && !down && left && right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_END_UP;
	}
	else if (!up && down && left && right)
	{
		block.corner = BLOCK_STATE_THIN_LINE_END_DOWN;
	}
	//corners
	else if (up && !down && left && !right)
	{
		block.corner = BLOCK_STATE_CORNER_UP_LEFT;
	}
	else if (up && !down && !left && right)
	{
		block.corner = BLOCK_STATE_CORNER_UP_RIGHT;
	}
	else if (!up && down && left && !right)
	{
		block.corner = BLOCK_STATE_CORNER_DOWN_LEFT;
	}
	else if (!up && down && !left && right)
	{
		block.corner = BLOCK_STATE_CORNER_DOWN_RIGHT;
	}

	return lastCorner != block.corner;//we have a change (or not)

}

