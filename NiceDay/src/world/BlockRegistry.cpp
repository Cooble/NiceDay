#include "ndpch.h"
#include "BlockRegistry.h"

//todo blockid will be position in list/vector

BlockRegistry::BlockRegistry()
{
}

void BlockRegistry::registerBlock(Block & block)
{
	m_blocks.push_back(block);
}

const Block& BlockRegistry::getBlock(int block_id)
{
	for (const Block& b : m_blocks) {
		if (b.getID() == block_id)
			return b;
	}
	//return m_blocks.at(0);
	ASSERT(false,"Invalid block_id!");

}


BlockRegistry::~BlockRegistry()
{
}
