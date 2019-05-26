#include "ndpch.h"
#include "World.h"
#include <utility>
#include "block/Block.h"
#include "WorldIO.h"
#include "block/BlockRegistry.h"
#include "biome/BiomeRegistry.h"
#include "block/block_datas.h"

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

World::World(std::string file_path, const char* name, int chunk_width, int chunk_height)
	:m_light_calc(this),
	m_info({ 0,chunk_width,chunk_height,0 }),
	m_file_path(std::move(file_path)),
	m_air_block(0)//seed,width,height,time
{
	strcpy_s(m_info.name, name);//name
	init();

}
World::World(std::string file_path, const WorldInfo * info)
	:m_light_calc(this),
	m_info(*info),
	m_file_path(std::move(file_path)),
	m_air_block(0)
{
	init();
}
World::~World() = default;

void World::genWorld(long seed)
{
	m_info.seed = seed;
	int surface_height = 40;
	for (int cx = 0; cx < m_info.chunk_width; cx++) {
		for (int cy = 0; cy < m_info.chunk_height; cy++) {
			loadChunk(cx, cy);
			Chunk& c = getChunk(cx, cy);
			c.m_biome = BIOME_UNDERGROUND;
			if ((cy + 1)*WORLD_CHUNK_SIZE > (m_info.terrain_level - 1))
				c.m_biome = BIOME_FOREST;
			if (cx < 2)
				c.m_biome = BIOME_DIRT;
			for (int x = 0; x < WORLD_CHUNK_SIZE; x++) {
				for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
				{

					auto& block = c.getBlock(x, y);
					
					auto worldy = cy * WORLD_CHUNK_SIZE + y;
					auto worldx = cx * WORLD_CHUNK_SIZE + x;
					int terrain = m_info.terrain_level + sin(worldx / 4) * 3;
					//	if (x == 0 || y == 0)
						//	block.id = BLOCK_AIR;
					if (worldy > terrain)
						block.block_id = BLOCK_AIR;
					else if (worldy == terrain)
						block.block_id = BLOCK_GRASS;
					else if (worldy > m_info.terrain_level - 7) {//5 block of dirt
						block.block_id = BLOCK_DIRT;
						block.setWall(WALL_DIRT);
					}
					else if (cx < 2)
						block.block_id = BLOCK_ADAMANTITE;
					else {
						block.block_id = BLOCK_STONE;
						block.setWall(WALL_STONE);
					}

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
	m_info.time++;
}

void World::saveAllChunks()
{
	for (auto& c : m_chunks)
		saveChunk(c);
}

Chunk& World::getChunk(int cx, int cy)
{
	int index = getChunkIndex(cx, cy);
	ASSERT(index != -1, "Invalid operation: retrieving chunk thats not loaded!");
	return m_chunks[index];
}

const Chunk* World::getLoadedChunkPointer(int cx, int cy) const
{
	int index = getChunkIndex(cx, cy);
	if (index == -1)
		return nullptr;
	return &m_chunks[index];
}

int World::getChunkIndex(int x, int y) const
{
	return getChunkIndex(Chunk::getChunkIDFromChunkPos(x, y));
}

int World::getChunkIndex(int id) const
{
	auto got = m_local_offset_map.find(id);
	if (got != m_local_offset_map.end()) {
		return got->second;
	}
	return -1;
}

Chunk& World::loadChunk(int x, int y)
{
	if (isChunkLoaded(x, y))
		return getChunk(x, y);
	for (auto i = 0; i < m_chunks.size(); i++)
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

void World::unloadChunks(std::set<int>& chunk_ids)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);

	for (int chunkId : chunk_ids) {
		auto& c = m_chunks[getChunkIndex(chunkId)];
		m_local_offset_map.erase(chunkId);//is complexity bad?
		int x, y;
		Chunk::getChunkPosFromID(chunkId, x, y);

		c.m_loaded = false;
		c.last_save_time = getTime();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
	}
}

void World::loadChunks(std::set<int>& chunk_ids)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);
	int lastFreeChunk = 0;
	for (int chunkId : chunk_ids) {
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
			lastFreeChunk = m_chunks.size() + 1;//dont bother scrolling through whole occupied chunklist
			//no unloaded chunk -> add new to list
			m_chunks.emplace_back();
			Chunk& c = m_chunks[m_chunks.size() - 1];

			stream.loadChunk(&c, getChunkSaveOffset(indexX, indexY));
			c.m_loaded = true;
			m_local_offset_map[c.chunkID()] = m_chunks.size() - 1;
		}
	}
}

void World::saveChunks(std::set<int>& chunk_ids)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);

	for (int chunkId : chunk_ids) {
		auto& c = m_chunks[getChunkIndex(chunkId)];
		c.last_save_time = getTime();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
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


bool World::isValidBlock(int x, int y) const
{
	return x >= 0 && y >= 0 && x < getInfo().chunk_width*WORLD_CHUNK_SIZE && y < getInfo().chunk_height*WORLD_CHUNK_SIZE;
}

const BlockStruct& World::getBlock(int x, int y) {
	return editBlock(x, y);
}

const BlockStruct* World::getBlockPointer(int x, int y) {
	if (isValidBlock(x, y))
	{
		auto& b = const_cast<BlockStruct&>(getBlock(x, y));
		return &b;
	}
	return nullptr;
}
const BlockStruct* World::getLoadedBlockPointer(int x, int y) const {
	int index = getChunkIndex(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (index != -1)
	{
		return &const_cast<BlockStruct&>(m_chunks[index].getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1)));
	}
	return nullptr;
}

BlockStruct& World::editBlock(int x, int y)
{
#ifdef ND_DEBUG
	if (!isValidBlock(x, y)) {
		m_air_block = 0;//when you edit this block next call to this function will return fresh air
		return m_air_block;
	}
#endif
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;

	int index = getChunkIndex(cx, cy);
	if (index == -1) {
		Chunk& c = loadChunk(cx, cy);

		return c.getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));
	}
	else
		return m_chunks[index].getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));
	return editBlock(x, y);
}

bool World::isAir(int x, int y) {
	return getBlock(x, y).isAir();
}

#define MAX_BLOCK_UPDATE_DEPTH 20

void World::onBlocksChange(int x, int y, int deep = 0)
{
	//todo fix bug when blocks at corners of the world dont set right corner
	deep++;
	if (deep >= MAX_BLOCK_UPDATE_DEPTH) {
		ND_WARN("Block update too deep! protecting stack");
		return;
	}
	if (isValidBlock(x, y + 1) && BlockRegistry::get().getBlock(getBlock(x, y + 1).block_id).onNeighbourBlockChange(this, x, y + 1)) {
		getChunk(World::getChunkCoord(x), World::getChunkCoord(y + 1)).markDirty(true);
		onBlocksChange(x, y + 1, deep);
	}

	if (isValidBlock(x, y - 1) && BlockRegistry::get().getBlock(getBlock(x, y - 1).block_id).onNeighbourBlockChange(this, x, y - 1)) {
		getChunk(World::getChunkCoord(x), World::getChunkCoord(y - 1)).markDirty(true);
		onBlocksChange(x, y - 1, deep);
	}

	if (isValidBlock(x + 1, y) && BlockRegistry::get().getBlock(getBlock(x + 1, y).block_id).onNeighbourBlockChange(this, x + 1, y)) {
		getChunk(World::getChunkCoord(x + 1), World::getChunkCoord(y)).markDirty(true);
		onBlocksChange(x + 1, y, deep);
	}

	if (isValidBlock(x - 1, y) && BlockRegistry::get().getBlock(getBlock(x - 1, y).block_id).onNeighbourBlockChange(this, x - 1, y)) {
		getChunk(World::getChunkCoord(x - 1), World::getChunkCoord(y)).markDirty(true);
		onBlocksChange(x - 1, y, deep);
	}

}

void World::onWallsChange(int xx, int yy, BlockStruct& blok)
{
	BlockRegistry::get().getWall(blok.wallID()).onNeighbourWallChange(this, xx, yy);
	/*for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			if (x == 0 && y == 0)
				continue;
			if (isValidBlock(x + xx, y + yy)) {
				BlockStruct& b = editBlock(x + xx, y + yy);
				if (b.isWallOccupied())//i cannot call this method on some foreign wall pieces
					BlockRegistry::get().getWall(b.wallID()).onNeighbourWallChange(this, x + xx, y + yy);
			}
		}
	}*/
	for (int x = -2; x < 3; ++x)
	{
		for (int y = -2; y < 3; ++y)
		{
			/*if (x >=-1&&x<2)
				continue;
			if (y >= -1 && y < 2)
				continue;*/
			if (x == 0 && y == 0)
				continue;
			if (isValidBlock(x + xx, y + yy)) {
				
				BlockStruct& b = editBlock(x + xx, y + yy);
				if (b.isWallOccupied()) {//i cannot call this method on some foreign wall pieces
					BlockRegistry::get().getWall(b.wallID()).onNeighbourWallChange(this, x + xx, y + yy);
					Chunk& c = getChunk(getChunkCoord(x + xx), getChunkCoord(y + yy));
					c.markDirty(true);//mesh needs to be updated
				}
			}
		}
	}

}

void World::setBlock(int x, int y, int block_id)
{
	BlockStruct s = block_id;
	setBlock(x, y, s);
}

void World::setWall(int x, int y, int wall_id)
{

#ifdef ND_DEBUG
	if (x < 0 || y < 0) {
		ND_WARN("Set block with invalid position");
		return;
	}
	if (x > getInfo().chunk_width*WORLD_CHUNK_SIZE - 1 || y > getInfo().chunk_height*WORLD_CHUNK_SIZE - 1) {
		ND_WARN("Set block with invalid position");
		return;
	}
#endif
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;

	int index = getChunkIndex(cx, cy);
	Chunk* c;
	if (index == -1)
		c = &loadChunk(cx, cy);
	else c = &m_chunks[index];

	//set block from old block acordingly
	auto& blok = c->getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));

	if (!blok.isWallOccupied() && wall_id == 0)//cannot erase other blocks parts on this shared block
		return;
	blok.setWall(wall_id);

	onWallsChange(x, y, blok);
	c->markDirty(true);

}


void World::setBlock(int x, int y, BlockStruct& blok)
{
#ifdef ND_DEBUG
	if (x < 0 || y < 0) {
		ND_WARN("Set block with invalid position");
		return;
	}
	if (x > getInfo().chunk_width*WORLD_CHUNK_SIZE - 1 || y > getInfo().chunk_height*WORLD_CHUNK_SIZE - 1) {
		ND_WARN("Set block with invalid position");
		return;
	}
#endif
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;

	int index = getChunkIndex(cx, cy);
	Chunk* c;
	if (index == -1)
		c = &loadChunk(cx, cy);
	else c = &m_chunks[index];

	//set walls from old block acordingly
	auto& block = c->getBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1));
	memcpy(blok.wall_id, block.wall_id, sizeof(blok.wall_id));
	memcpy(blok.wall_corner, block.wall_corner, sizeof(blok.wall_corner));

	c->setBlock(x & (WORLD_CHUNK_SIZE - 1), y  & (WORLD_CHUNK_SIZE - 1), blok);
	BlockRegistry::get().getBlock(blok.block_id).onNeighbourBlockChange(this, x, y);
	onBlocksChange(x, y);
	c->markDirty(true);
}


