#include "ndpch.h"
#include "BlockRegistry.h"

//todo blockid will be position in list/vector

BlockRegistry::BlockRegistry()
{
}

void BlockRegistry::registerBlock(Block* block)
{
	m_blocks.resize(block->getID()+1);
	m_blocks[block->getID()] = block;
}

const Block& BlockRegistry::getBlock(int block_id)
{
	ASSERT(m_blocks.size() > block_id, "Invalid block id");
	return *m_blocks[block_id];
}


BlockRegistry::~BlockRegistry()
{
	for (Block* b : m_blocks)
		delete b;
}
