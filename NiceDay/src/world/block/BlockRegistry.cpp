#include "ndpch.h"
#include "BlockRegistry.h"


void BlockRegistry::registerBlock(Block* block)
{
	m_blocks.resize(block->getID()+1);
	m_blocks[block->getID()] = block;
}

void BlockRegistry::registerWall(Wall* wall)
{
	m_walls.resize(wall->getID()+1);
	m_walls[wall->getID()] = wall;
}

const Block& BlockRegistry::getBlock(int block_id)
{
	ASSERT(m_blocks.size() > block_id, "Invalid block id");
	return *m_blocks[block_id];
}

const Wall& BlockRegistry::getWall(int wall_id)
{
	ASSERT(m_walls.size() > wall_id, "Invalid wall id");
	return *m_walls[wall_id];
}


BlockRegistry::~BlockRegistry()
{
	for (auto b : m_blocks)
		delete b;
	for (auto b : m_walls)
		delete b;
}
