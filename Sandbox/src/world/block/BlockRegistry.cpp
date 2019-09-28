#include "ndpch.h"
#include "BlockRegistry.h"


void BlockRegistry::initTextures(const BlockTextureAtlas& atlas)
{
	for (Block* block : m_blocks)
	{
		block->onTextureLoaded(atlas);
	}
	for (Wall* wall : m_walls)
	{
		wall->onTextureLoaded(atlas);
	}
}

void BlockRegistry::registerBlock(Block* block)
{
	if(m_blocks.size()<=block->getID())
		m_blocks.resize(block->getID()+1);
	m_blocks[block->getID()] = block;
	m_blockIDs[block->toString()] = block->getID();
}

void BlockRegistry::registerWall(Wall* wall)
{
	if (m_walls.size() <= wall->getID())
		m_walls.resize(wall->getID()+1);
	m_walls[wall->getID()] = wall;
	m_wallIDs[wall->toString()] = wall->getID();

}

const Block& BlockRegistry::getBlock(int block_id)
{
	ASSERT(m_blocks.size() > block_id&&block_id>=0, "Invalid block id");
	return *m_blocks[block_id];
}

const Wall& BlockRegistry::getWall(int wall_id)
{
	ASSERT(m_walls.size() > wall_id&&wall_id >= 0, "Invalid wall id");
	return *m_walls[wall_id];
}

const Block& BlockRegistry::getBlock(const std::string& block_id) const
{
	return *m_blocks[m_blockIDs.at(block_id)];
}

const Wall& BlockRegistry::getWall(const std::string& wall_id)const
{
	return *m_walls[m_wallIDs.at(wall_id)];
}


BlockRegistry::~BlockRegistry()
{
	for (auto b : m_blocks)
		delete b;
	for (auto b : m_walls)
		delete b;
}
