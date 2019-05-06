#include "ndpch.h"
#include "Block.h"
#include "world/World.h"


Block::Block(int id) : m_id(id)
{
}

Block::~Block()
{
}

int Block::getTextureOffset(int metadata) const
{
	return 0;
}

int Block::getCornerOffset(int corner_state) const
{
	return corner_state;
}

bool Block::onNeighbourBlockChange(World* world, int x, int y) const
{
	if (getID() == BLOCK_AIR)
		return false;

	BlockStruct& upB = world->getBlock(x, y + 1);
	BlockStruct& downB = world->getBlock(x, y - 1);
	BlockStruct& leftB = world->getBlock(x - 1, y);
	BlockStruct& rightB = world->getBlock(x + 1, y);

	bool up = upB.isAir();
	bool down = downB.isAir();
	bool left = leftB.isAir();
	bool right = rightB.isAir();


	BlockStruct& block = world->getBlock(x, y);
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

