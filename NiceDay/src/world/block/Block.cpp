#include "ndpch.h"
#include "world/World.h"
#include "Block.h"


Block::Block(int id)
	:m_id(id), m_opacity(0.25f), m_texture_offset(0)
{
}

Block::~Block() = default;

float Block::getOpacity(const BlockStruct&) const
{
	return m_opacity;
}

int Block::getTextureOffset(const BlockStruct&)const
{
	return m_texture_offset;
}

int Block::getCornerOffset(const BlockStruct& b) const
{
	return b.corner;
}

bool Block::onNeighbourBlockChange(World* world, int x, int y) const
{
	if (getID() == BLOCK_AIR)
		return false;

	bool up = world->isAir(x, y + 1);
	bool down = world->isAir(x, y - 1);
	bool left = world->isAir(x - 1, y);
	bool right = world->isAir(x + 1, y);

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

