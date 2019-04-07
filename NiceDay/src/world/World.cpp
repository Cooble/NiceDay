#include "ndpch.h"
#include "World.h"


World::World(): 
	m_blocks(nullptr)
{

}


World::~World()
{
	delete m_blocks;
}

void World::genWorld() {
	m_blocks = (BlockStruct*)malloc(sizeof(BlockStruct)*m_size);
	for (int i = 0; i < m_size; i++)
		m_blocks[i] = BlockStruct();
	
}
