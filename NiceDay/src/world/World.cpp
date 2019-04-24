#include "ndpch.h"
#include "World.h"
#include "WorldGen.h"

#include <algorithm>

Chunk::Chunk() :m_loaded(false)
{
}

World::World(std::string name, int chunk_width, int chunk_height) :
	m_name(name), m_chunk_width(chunk_width), m_chunk_height(chunk_height)
{
	m_chunks.reserve(CHUNK_BUFFER_LENGTH);
}
World::~World()
{
}

void World::genWorld(long seed)
{
	m_seed = seed;
	int surface_height = 100;
	for (int cx = 0; cx < m_chunk_width; cx++) {
		for (int cy = 0; cy < m_chunk_height; cy++) {
			for (int x = 0; x < WORLD_CHUNK_SIZE; x++) {
				for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
				{
					//does it copy or by reference?
					auto block = getBlock(cx*WORLD_CHUNK_SIZE + x, cy*WORLD_CHUNK_SIZE + y);
					if (y < surface_height)
						block.id = 1;
					else block.id = 0;
				}
			}
			unloadChunk(cx, cy);
		}

	}
}

void World::onUpdate()
{
}

void World::onRender()
{

}

long World::getTime()
{
	return 0;
}

Chunk& World::getChunk(int x, int y)
{
	int index = getChunkIndex(x, y);
	ASSERT(index != -1, "Invalid operation retrieving chunk thats not loaded!");
	return m_chunks[index];
}

int World::getChunkIndex(int x, int y)
{
	for (int i = 0; i < m_chunks.size(); i++) {
		Chunk& c = m_chunks[i];
		if (c.isLoaded() && c.m_x == x && c.m_y == y) {
			return i;
		}
	}
	return -1;
}

void World::genChunk(int x, int y)
{
}

Chunk& World::loadChunk(int x, int y)
{
	for (int i = 0; i < CHUNK_BUFFER_LENGTH; i++)
	{
		Chunk& c = m_chunks[i];
		if (!c.isLoaded())
		{
			WorldGen::loadChunk(m_name, &c, getChunkOffset(x, y));
			c.m_loaded = true;
			return c;
		}
	}
	//no unloaded chunk -> add new to list
	m_chunks.emplace_back();
	Chunk& c = m_chunks[m_chunks.size() - 1];

	//memcopy from file to blocks
	WorldGen::loadChunk(m_name, &c, getChunkOffset(x, y));
	c.m_loaded = true;
	return c;

}

void World::unloadChunk(int x, int y)
{
	int index = getChunkIndex(x, y);
	if (index != -1) {
		Chunk& c = m_chunks[index];
		if (c.isLoaded() && c.m_x == x && c.m_y == y)
		{
			c.m_loaded = false;
			c.timestamp = getTime();
			WorldGen::saveChunk(m_name, &c, getChunkOffset(x, y));
			return;
		}
	}
	ND_WARN("Unloading chunk that is not loaded!");
}


BlockStruct& World::getBlock(int x, int y) {
	int cx = x / WORLD_CHUNK_SIZE;
	int cy = y / WORLD_CHUNK_SIZE;
	int index = getChunkIndex(cx, cy);
	if (index == -1) {
		Chunk& c = loadChunk(cx, cy);
		return c.getBlock(x % WORLD_CHUNK_SIZE, y % WORLD_CHUNK_SIZE);
	}
	else
		return m_chunks[index].getBlock(x % WORLD_CHUNK_SIZE, y % WORLD_CHUNK_SIZE);
}


