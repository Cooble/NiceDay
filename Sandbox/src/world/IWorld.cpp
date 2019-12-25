#include "IWorld.h"
#include "block/Block.h"

void IWorld::setBlock(int wx, int wy, int blockid)
{
	setBlock(wx, wy, BlockStruct(blockid));
		
}

void IWorld::setWall(int wx, int wy, int wallid)
{
	BlockStruct b;
	b.setWall(wallid);
	setWall(wx, wy,b);
}
