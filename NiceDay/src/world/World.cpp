#include "ndpch.h"
#include "World.h"
#include "WorldIO.h"

#include <algorithm>

Chunk::Chunk() :m_loaded(false)
{
}
void World::init() {
	m_chunks.reserve(CHUNK_BUFFER_LENGTH);
	for (int i = 0; i < CHUNK_BUFFER_LENGTH; i++) {
		m_chunks.emplace_back();
		//m_chunks.at(i).m_loaded = false; redundant
	}
	ND_INFO("Made world instance");
}

World::World(const std::string& filePath, const char* name, int chunk_width, int chunk_height)
	:m_info({ 0,chunk_width,chunk_height,0 }), m_file_path(filePath)//seed,width,height,time
{//todo check if m_file_path works?
	strcpy_s(m_info.name, name);//name
	init();

}
World::World(const std::string& filePath, const WorldInfo * info)
	:m_info(*info), m_file_path(filePath)
{
	init();
}
World::~World()
{
}

void World::genWorld(long seed)
{
	m_info.seed = seed;
	int surface_height = 100;
	for (int cx = 0; cx < m_info.chunk_width; cx++) {
		for (int cy = 0; cy < m_info.chunk_height; cy++) {
			loadChunk(cx, cy);
			Chunk& c = getChunk(cx, cy);
			for (int x = 0; x < WORLD_CHUNK_SIZE; x++) {
				for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
				{
					auto block = c.getBlock(x, y);
					if (y < surface_height)
						block.id = 1;
					else block.id = 0;
				}
			}
			unloadChunk(c);
		}
	}
}

void World::onUpdate()
{
}

void World::tick()
{
}

void World::onRender()
{

}


void World::unloadAllChunks()
{
	for (Chunk& c : m_chunks) {
		unloadChunk(c);
	}
}

void World::saveAllChunks()
{
	for (Chunk& c : m_chunks) {
		saveChunk(c);
	}
}

Chunk& World::getChunk(int x,int y)
{
	int index = getChunkIndex(x,y);
	ASSERT(index != -1, "Invalid operation retrieving chunk thats not loaded!");
	return m_chunks[index];
}

int World::getChunkIndex(int x,int y)
{
	return getChunkIndex(Chunk::getChunkIDFromChunkPos(x, y));
}
int World::getChunkIndex(int id)
{
	auto got = m_local_offset_map.find(id);
	if (got != m_local_offset_map.end()) {
		return got->second;
	}
	return -1;
}

Chunk& World::loadChunk(int x,int y)
{
	for (int i = 0; i < m_chunks.size(); i++)
	{
		Chunk& c = m_chunks[i];
		if (!c.isLoaded())
		{
			WorldIO::Session stream = WorldIO::Session(m_file_path, false);
			stream.loadChunk(&c, getChunkSaveOffset(x, y));
			c.m_loaded = true;
			m_local_offset_map[c.chunkID()] = i;
			return c;
		}
	}
	//no unloaded chunk -> add new to list
	m_chunks.emplace_back();
	Chunk& c = m_chunks[m_chunks.size() - 1];

	WorldIO::Session stream = WorldIO::Session(m_file_path, false);
	stream.loadChunk(&c, getChunkSaveOffset(x, y));
	c.m_loaded = true;
	m_local_offset_map[c.chunkID()] = m_chunks.size() - 1;

	return c;

}

void World::unloadChunk(Chunk& c)
{
	if (c.isLoaded())
	{
		m_local_offset_map.erase(c.chunkID());//is complexity bad?
		c.m_loaded = false;
		c.last_save_time = getTime();
		WorldIO::Session stream = WorldIO::Session(m_file_path, true);
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
		return;
	}
	ND_INFO("canot remove chunk " + m_chunks.size());
}

void World::unloadChunks(std::set<int>& chunkIds)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);

	for (int chunkId : chunkIds) {
		auto& c = m_chunks[getChunkIndex(chunkId)];
		m_local_offset_map.erase(chunkId);//is complexity bad?
		int x, y;
		Chunk::getChunkPosFromID(chunkId, x, y);

		c.m_loaded = false;
		c.last_save_time = getTime();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
	}
}

void World::loadChunks(std::set<int>& chunkIds)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);
	int lastFreeChunk = 0;
	for (int chunkId : chunkIds) {
		int indexX, indexY;
		Chunk::getChunkPosFromID(chunkId, indexX, indexY);
		bool foundFreeChunk = false;
		for (int i = lastFreeChunk; i < m_chunks.size(); i++)
		{
			Chunk& c = m_chunks[i];
			if (!c.isLoaded())
			{
				lastFreeChunk = i + 1;
				stream.loadChunk(&c, getChunkSaveOffset(indexX, indexY));
				c.m_loaded = true;
				m_local_offset_map[c.chunkID()] = i;
				foundFreeChunk = true;
				break;
			}
		}
		if (!foundFreeChunk) {
			lastFreeChunk = m_chunks.size();//dont bother scrolling through whole occupied chunklist
			//no unloaded chunk -> add new to list
			m_chunks.emplace_back();
			Chunk& c = m_chunks[m_chunks.size() - 1];

			stream.loadChunk(&c, getChunkSaveOffset(indexX, indexY));
			c.m_loaded = true;
			m_local_offset_map[c.chunkID()] = m_chunks.size() - 1;
		}
	}
}

void World::saveChunks(std::set<int>& chunkIds)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);

	for (int chunkId : chunkIds) {
		auto& c = m_chunks[getChunkIndex(chunkId)];
		c.last_save_time = getTime();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
		return;

		ND_INFO("canot remove chunk " + m_chunks.size());
	}
}

void World::saveChunk(Chunk& c)
{
	if (c.isLoaded()) {
		c.last_save_time = getTime();
		WorldIO::Session stream = WorldIO::Session(m_file_path, true);
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
	}
}


BlockStruct& World::getBlock(int x, int y) {
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;

	int index = getChunkIndex(cx, cy);
	if (index == -1) {
		Chunk& c = loadChunk(cx, cy);

		return c.getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));
	}
	else
		return m_chunks[index].getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));
}


