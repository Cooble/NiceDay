#include "ndpch.h"
#include "BlockAccess.h"
#include "World.h"

void BlockAccess::setBlock(int x, int y, int blockid)
{
	setBlock(x, y, BlockStruct(blockid));
}

void BlockAccess::setBlockWithNotify(int x, int y, int blockid)
{
	setBlockWithNotify(x, y, BlockStruct(blockid));
}
