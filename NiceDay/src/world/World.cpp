#include "ndpch.h"
#include <utility>
#include "block/Block.h"
#include "WorldIO.h"
#include "block/BlockRegistry.h"
#include "biome/BiomeRegistry.h"
#include "Game.h"
#include "entity/EntityRegistry.h"
#include <filesystem>



Chunk::Chunk()
{
	setLoaded(false);
	lock(false);
}

void World::init()
{
	m_chunks.reserve(CHUNK_BUFFER_LENGTH);
	for (int i = 0; i < CHUNK_BUFFER_LENGTH; i++)
	{
		m_chunks.emplace_back();
		//m_chunks.at(i).m_loaded = false; redundant
	}
	ND_INFO("Made world instance");
}

World::World(std::string file_path, const WorldInfo& info)
	: m_light_calc(this),
	  m_info(info),
	  m_file_path(file_path),
	  m_nbt_saver(file_path + ".entity")
{
	init();
}

World::~World() = default;

static bool taskActive = false;
static std::queue<std::set<int>> loadQueue;


void World::onUpdate()
{
	tick();
	if (!taskActive && !loadQueue.empty())
	{
		taskActive = true;
		genChunks(loadQueue.front());
		loadQueue.pop();
	}
}
static int light_tick_delay=0;
constexpr int MAX_LIGHT_TICK_DELAY=60;
void World::tick()
{

	m_info.time++;

	//we need a buffer here cause we cant modify array during iteration
	if (m_entity_array_buff.size() != m_entity_array.size())
		m_entity_array_buff.resize(m_entity_array.size()); //make sure there is enough space in buff
	memcpy(m_entity_array_buff.data(), m_entity_array.data(), m_entity_array.size() * sizeof(EntityID));

	for (int i = m_entity_array_buff.size() - 1; i >= 0; --i)
	{
		WorldEntity* entity = m_entity_manager.entity(m_entity_array_buff[i]);
		if (entity)
			entity->update(this);
	}
	/*if(light_tick_delay++==MAX_LIGHT_TICK_DELAY)
	{
		light_tick_delay = 0;
		for (auto& chunk : m_chunks)
		{
			if(!chunk.isLoaded())
				continue;
			auto& biome = BiomeRegistry::get().getBiome(chunk.getBiome());
			if(biome.hasDynamicLighting())
			{
				
			}
		}
	}*/
}


constexpr float minLight = 0.13f;

constexpr float startRiseHour = 9.5f;
constexpr float endRiseHour = 11.f;

constexpr float startSetHour = 15.5f;
constexpr float endSetHour = 16.25f;

static float smootherstep(float x) {
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

glm::vec4 World::getSkyColor()
{
	auto hour = getWorldTime().hours();
	float f = 0;
	if(hour>=startRiseHour&&hour<endRiseHour)
	{
		hour -= startRiseHour;
		hour /= (endRiseHour - startRiseHour);
		f = minLight + smootherstep(hour)*(1-minLight);
	}
	else if(hour >= startSetHour && hour < endSetHour)
	{
		hour -= startSetHour;
		hour /= (endSetHour - startSetHour);
		f = minLight + (1 - smootherstep(hour))*(1 - minLight);
	}
	else if (hour >= endRiseHour&&hour<startSetHour)
		f = 1;
	else f = minLight;
	return glm::vec4( 0,0,0,f);

}

bool World::isChunkGenerated(int x, int y)
{
	if (!isChunkValid(x, y))
		return false;
	if (isChunkLoaded(x, y))
		return getChunk(x, y).isGenerated();

	//todo dont load whole just just to see if it has been generated
	auto stream = WorldIO::Session(m_file_path, true);

	int chunkIndex = getNextFreeChunkIndex(0);

	Chunk& c = m_chunks[chunkIndex];
	int saveOffset = getChunkSaveOffset(x, y);
	stream.loadChunk(&c, saveOffset);
	bool isGen = c.isGenerated();
	c.setLoaded(false);
	stream.close();
	return isGen;
}

Chunk& World::getChunk(int cx, int cy)
{
	int index = getChunkIndex(cx, cy);
	ASSERT(index != -1, "Invalid operation: retrieving chunk thats not loaded! {}, {}", cx, cy);
	if (index == -1)
		for (auto& d : m_local_offset_map)
		{
			ND_TRACE("[{},{},\t- {}]", half_int::getX(d.first), half_int::getY(d.first));
		}

	return m_chunks[index];
}

const Chunk* World::getLoadedChunkPointer(int cx, int cy) const
{
	int index = getChunkIndex(cx, cy);
	if (index == -1)
		return nullptr;
	return &m_chunks[index];
}

Chunk* World::getLoadedChunkPointerNoConst(int cx, int cy) //todo really rename it man its terrible
{
	int index = getChunkIndex(cx, cy);
	if (index == -1)
		return nullptr;
	return &m_chunks[index];
}

int World::getChunkIndex(int x, int y) const
{
	return getChunkIndex(half_int(x, y));
}

int World::getChunkIndex(int id) const
{
	auto got = m_local_offset_map.find(id);
	if (got != m_local_offset_map.end())
		return got->second;
	return -1;
}

Chunk& World::loadChunk(int x, int y)
{
	std::set<int> s;
	s.insert(half_int(x, y));
	loadChunksAndGen(s);
	return getChunk(x, y);
}


void World::loadChunksAndGen(std::set<int>& toLoadChunks)
{
	//todo problem with chunk lock, loadchunks is called before task finishes -> some task unlocks it faster than second
	auto stream = WorldIO::Session(m_file_path, true);
	m_nbt_saver.beginSession();

	std::set<int> toGenChunks;

	int lastFreeChunk = 0;
	for (int chunkId : toLoadChunks) //need load everything and lock it because genTask will be working on it
	{
		int x = half_int::getX(chunkId);
		int y = half_int::getY(chunkId);

		ASSERT(isChunkValid(x, y), "Invalid chunk pos");

		if (isChunkLoaded(x, y))
			continue;

		int chunkIndex = getNextFreeChunkIndex(lastFreeChunk);
		lastFreeChunk = chunkIndex + 1;

		Chunk& c = m_chunks[chunkIndex];
		int saveOffset = getChunkSaveOffset(x, y);
		stream.loadChunk(&c, saveOffset);
		m_local_offset_map[c.chunkID()] = chunkIndex;
		c.setLoaded(true);
		if (!c.isGenerated())
		{
			//fresh chunk -> need to generate it
			toGenChunks.insert(half_int(x, y));
			//ND_INFO("GEN {}, {}", x, y);
		}
		else
		{
			//need to load entities

			if(m_nbt_saver.setReadChunkID(c.chunkID()))
			{
				uint32_t numberOfEntities = 0;
				m_nbt_saver.read(numberOfEntities);
				for (int i = 0; i < numberOfEntities; ++i)
				{
					NBT n;
					n.deserialize(&m_nbt_saver);
					auto entity= EntityRegistry::get().loadInstance(n);
					loadEntity(entity);
				}
			}
			//ND_INFO("LOAD {}, {}", x, y);
		}
	}
	m_nbt_saver.endSession();

	if (!toGenChunks.empty())
		loadQueue.push(toGenChunks);
}

constexpr int maskUp = BIT(0);
constexpr int maskDown = BIT(1);
constexpr int maskLeft = BIT(2);
constexpr int maskRight = BIT(3);

void World::genChunks(std::set<int>& toGenChunks)
{
	defaultable_map<int, int, 0> toupdateChunks;
	for (auto chunkId : toGenChunks)
	{
		int x = half_int::getX(chunkId);
		int y = half_int::getY(chunkId);

		toupdateChunks[half_int(x, y)] = maskRight | maskLeft | maskUp | maskDown;
		//yes we need to update even this one
		toupdateChunks[half_int(x + 1, y)] |= maskLeft;
		toupdateChunks[half_int(x - 1, y)] |= maskRight;
		toupdateChunks[half_int(x, y + 1)] |= maskDown;
		toupdateChunks[half_int(x, y - 1)] |= maskUp;

		//add chunks that need to be loaded but wont be changed
		toupdateChunks[half_int(x - 1, y - 1)] |= 0;
		toupdateChunks[half_int(x - 1, y + 1)] |= 0;
		toupdateChunks[half_int(x + 1, y - 1)] |= 0;
		toupdateChunks[half_int(x + 1, y + 1)] |= 0;
	}

	//load section============================================================================
	//load all needed chunks and remove those which are not generated (but toGenChunks)
	auto stream = WorldIO::Session(m_file_path, true);
	int lastFreeChunk = 0;
	std::vector<int> toErase;
	for (auto chunkId : toupdateChunks)
	{
		int id = chunkId.first;
		int x = half_int::getX(id);
		int y = half_int::getY(id);
		if (!isChunkValid(x, y))
		{
			toErase.push_back(id);
			continue;
		}

		int index = getChunkIndex(id);
		if (index == -1)
		{
			index = getNextFreeChunkIndex(lastFreeChunk);
			lastFreeChunk = index + 1;

			Chunk& c = m_chunks[index];
			int saveOffset = getChunkSaveOffset(x, y);
			stream.loadChunk(&c, saveOffset);
			//checking if we have not-generated chunk that is not set to be generated
			if (!c.isGenerated() && toGenChunks.find(id) == toGenChunks.end())
			{
				c.setLoaded(false);
				//ND_INFO("erasing {}, {}",x,y);
				toErase.push_back(id);
				continue;
			}
			else
			{
				m_local_offset_map[c.chunkID()] = index;
				c.setLoaded(true);
			}
		}
		m_chunks[index].lock(true);
	}
	for (int id : toErase) //remove invalid and nevergenerated (excluding toGenChunks)
		toupdateChunks.erase(toupdateChunks.find(id));
	//load section done============================================================================

	int genIndex = 0;
	int upIndex = 0;

	Game::get().getScheduler().runTaskTimer(
		[genIndex, upIndex, this, toGenChunks, toupdateChunks]() mutable -> bool
		{
			taskActive = true;
			int numberOfGens = 0;
			auto genIt = toGenChunks.begin();
			std::advance(genIt, genIndex);
			while (genIt != toGenChunks.end())
			{
				Chunk* c = &m_chunks[getChunkIndex(*genIt)];

				this->m_gen.gen(this->m_info.seed, this, *c);
				
				c->lightLock();
				this->m_light_calc.assignComputeChunk(c->getCX(), c->getCY());

				++genIndex;
				++genIt;
				++numberOfGens;

				if (numberOfGens >= 1)
					return false;
			}

			int numberOfUps = 0;
			auto upIt = toupdateChunks.begin();
			std::advance(upIt, upIndex);
			while (upIt != toupdateChunks.end())
			{
				int id = (*upIt).first;
				int mask = (*upIt).second;
				auto& c = m_chunks[getChunkIndex(id)];

				auto resources = LightCalculator::createQuadroCross(c.getCX(), c.getCY());
				for (int i = 0; i < 4; ++i) {
					auto pC = getLoadedChunkPointerNoConst(resources[i].first, resources[i].second);
					if (pC)
						pC->lightLock();
				}
				
				m_light_calc.assignComputeChunkBorders(c.getCX(),c.getCY());
				if (mask != 0)//check if chunk is not only for readonly purposes
				{
					this->updateChunkBounds(half_int::getX(id), half_int::getY(id), mask);
				}
				++upIndex;
				++upIt;
				++numberOfUps;
				if (numberOfUps >= 1)
				{
					return false;
				}
			}

			for (auto& i : toupdateChunks)
			{
				Chunk& c = this->getChunk(half_int::getX(i.first), half_int::getY(i.first));
				c.lock(false); //unlock chunks so they can be unloaded
				c.markDirty(true); //need render after change
			}
			//ND_INFO("we are done with {} chunks", toupdateChunks.size());
			taskActive = false;
			return true;
		}, 1);
}

void World::loadLightResources(int x, int y)
{
	auto resources = LightCalculator::computeQuadroSquare(x, y);
	for (int i = 0; i < 4; ++i)
	{
		auto p = resources[i];
		if (isChunkValid(p.first, p.second))
		{
			if (!isChunkLoaded(p.first, p.second))
			{
				loadChunk(p.first, p.second);
			}
			//if(!getChunk(p.first,p.second).isLightLocked())
			//	ND_INFO("Lightlocked chunk: {},{} :", p.first, p.second, getChunk(p.first, p.second).isLightLocked());

			getChunk(p.first, p.second).lightLock();
		}
	}
}

int World::getNextFreeChunkIndex(int startSearchIndex)
{
	int out = -1;
	for (; startSearchIndex < m_chunks.size(); startSearchIndex++) //check if some chunk is free
	{
		if (!m_chunks[startSearchIndex].isLoaded())
		{
			out = startSearchIndex;
			break;
		}
	}
	if (out == -1)
	{
		m_chunks.reserve(m_chunks.size() + 1);
		m_chunks.emplace_back();
		out = m_chunks.size() - 1;
	}
	return out;
}

void World::updateChunkBounds(int xx, int yy, int bitBounds)
{
	Chunk& c = getChunk(xx, yy);
	int wx = c.m_x * WORLD_CHUNK_SIZE;
	int wy = c.m_y * WORLD_CHUNK_SIZE;
	if ((bitBounds & maskUp) != 0)
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			auto& block = c.block(x, WORLD_CHUNK_SIZE - 1);
			auto worldx = wx + x;
			auto worldy = wy + WORLD_CHUNK_SIZE - 1;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(this, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(this, worldx, worldy);
		}
	if ((bitBounds & maskDown) != 0)
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			auto& block = c.block(x, 0);
			auto worldx = wx + x;
			auto worldy = wy;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(this, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(this, worldx, worldy);
		}
	if ((bitBounds & maskLeft) != 0)
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.block(0, y);
			auto worldx = wx;
			auto worldy = wy + y;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(this, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(this, worldx, worldy);
		}
	if ((bitBounds & maskRight) != 0)
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.block(WORLD_CHUNK_SIZE - 1, y);
			auto worldx = wx + WORLD_CHUNK_SIZE - 1;
			auto worldy = wy + y;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(this, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(this, worldx, worldy);
		}
}

void World::unloadChunk(Chunk& c)
{
	std::set<int> s;
	s.insert(c.chunkID());
	unloadChunks(s);
}

void World::unloadChunks(std::set<int>& chunk_ids)
{
	WorldIO::Session stream = WorldIO::Session(m_file_path, true);
	m_nbt_saver.beginSession();
	std::vector<EntityID> toUnload;

	for (int chunkId : chunk_ids)
	{
		auto& c = m_chunks[getChunkIndex(chunkId)];
		ASSERT(c.isLoaded(), "unloading not loaded chunks");
		if (!c.isLoaded())
			continue;
		auto rect = c.getChunkRectangle();
		//ND_INFO("Unloaded chunk {},{}", half_int(chunkId).x, half_int(chunkId).y);
		m_local_offset_map.erase(chunkId); //is complexity bad?
		c.setLoaded(false);
		c.last_save_time = getWorldTicks();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));
		for (EntityID id : m_entity_array)
		{
			auto e = m_entity_manager.entityDangerous(id);
			ASSERT(m_entity_manager.isLoaded(id) && m_entity_manager.isAlive(id), "Shit");
			if(rect.containsPoint(e->getPosition()))
				toUnload.push_back(id);
		}
		m_nbt_saver.setWriteChunkID(chunkId);
		uint32_t entityCount = toUnload.size();
		if(entityCount)
			m_nbt_saver.write(entityCount);//if chunk is blank no need to write anything
		NBT nbt;
		for (auto entity : toUnload)
		{
			m_entity_manager.entity(entity)->save(nbt);
			nbt.serialize(&m_nbt_saver);
			nbt.clear();

			unloadEntity(entity);
		}
		m_nbt_saver.flushWrite();
		toUnload.clear();

	}
	m_nbt_saver.endSession();
}

const BlockStruct* World::getBlockPointer(int x, int y)
{
	if (isBlockValid(x, y))
	{
		auto& b = const_cast<BlockStruct&>(getBlock(x, y));
		return &b;
	}
	return nullptr;
}

const BlockStruct* World::getLoadedBlockPointer(int x, int y) const
{
	int index = getChunkIndex(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE);
	if (index != -1)
	{
		return &const_cast<BlockStruct&>(m_chunks[index].block(x & (WORLD_CHUNK_SIZE - 1),
		                                                          y & (WORLD_CHUNK_SIZE - 1)));
	}
	return nullptr;
}

void World::flushBlockSet()
{
	m_edit_buffer_enable = false;
	while (!m_edit_buffer.empty())
	{
		auto pair = m_edit_buffer.front();
		m_edit_buffer.pop();
		BlockRegistry::get().getBlock(getBlock(pair.first, pair.second).block_id).onNeighbourBlockChange(
			this, pair.first, pair.second);
		onBlocksChange(pair.first, pair.second);

		//todo this is big bullshit we will need to separate it somehow to update in batches
		loadLightResources(pair.first, pair.second);
		m_light_calc.assignComputeChange(pair.first, pair.second); //refresh light around the block
	}
}

BlockStruct& World::editBlock(int x, int y)
{
#ifdef ND_DEBUG
	if (!isBlockValid(x, y))
	{
		m_air_block = 0; //when you edit this block next call to this function will return fresh air
		return m_air_block;
	}
#endif
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;

	int index = getChunkIndex(cx, cy);
	if (index == -1)
	{
		Chunk& c = loadChunk(cx, cy);

		return c.block(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	}
	else
		return m_chunks[index].block(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
}

bool World::isAir(int x, int y)
{
	if (!isBlockValid(x, y))
		return true;
	return getBlock(x, y).isAir();
}

#define MAX_BLOCK_UPDATE_DEPTH 20

void World::onBlocksChange(int x, int y, int deep)
{
	//todo fix bug when blocks at corners of the world dont set right corner
	++deep;
	if (deep >= MAX_BLOCK_UPDATE_DEPTH)
	{
		ND_WARN("Block update too deep! protecting stack");
		return;
	}
	if (isBlockValid(x, y + 1) && BlockRegistry::get()
	                              .getBlock(getBlock(x, y + 1).block_id).onNeighbourBlockChange(this, x, y + 1))
	{
		getChunk(World::toChunkCoord(x), World::toChunkCoord(y + 1)).markDirty(true);
		onBlocksChange(x, y + 1, deep);
	}

	if (isBlockValid(x, y - 1) && BlockRegistry::get()
	                              .getBlock(getBlock(x, y - 1).block_id).onNeighbourBlockChange(this, x, y - 1))
	{
		getChunk(World::toChunkCoord(x), World::toChunkCoord(y - 1)).markDirty(true);
		onBlocksChange(x, y - 1, deep);
	}

	if (isBlockValid(x + 1, y) && BlockRegistry::get()
	                              .getBlock(getBlock(x + 1, y).block_id).onNeighbourBlockChange(this, x + 1, y))
	{
		getChunk(World::toChunkCoord(x + 1), World::toChunkCoord(y)).markDirty(true);
		onBlocksChange(x + 1, y, deep);
	}

	if (isBlockValid(x - 1, y) && BlockRegistry::get()
	                              .getBlock(getBlock(x - 1, y).block_id).onNeighbourBlockChange(this, x - 1, y))
	{
		getChunk(World::toChunkCoord(x - 1), World::toChunkCoord(y)).markDirty(true);
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
			if (isBlockValid(x + xx, y + yy))
			{
				BlockStruct& b = editBlock(x + xx, y + yy);
				if (b.isWallOccupied())
				{
					//i cannot call this method on some foreign wall pieces
					BlockRegistry::get().getWall(b.wallID()).onNeighbourWallChange(this, x + xx, y + yy);
					Chunk& c = getChunk(toChunkCoord(x + xx), toChunkCoord(y + yy));
					c.markDirty(true); //mesh needs to be updated
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
	if (x < 0 || y < 0)
	{
		ND_WARN("Set block with invalid position");
		return;
	}
	if (x > getInfo().chunk_width * WORLD_CHUNK_SIZE - 1 || y > getInfo().chunk_height * WORLD_CHUNK_SIZE - 1)
	{
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
	auto& blok = c->block(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));

	if (!blok.isWallOccupied() && wall_id == 0) //cannot erase other blocks parts on this shared block
		return;
	blok.setWall(wall_id);
	onWallsChange(x, y, blok);
	//if (BiomeRegistry::get().getBiome(c->getBiome()).getBackgroundLight() != 0)
	loadLightResources(x, y);
	m_light_calc.assignComputeChange(x, y);

	c->markDirty(true);
}

EntityID World::spawnEntity(WorldEntity* pEntity)
{
	auto id = m_entity_manager.createEntity();
	pEntity->m_id = id;
	loadEntity(pEntity);
	return id;
}

void World::unloadEntity(EntityID worldEntity)
{
	for (int i = 0; i < m_entity_array.size(); ++i)
	{
		if (m_entity_array[i] == worldEntity)
		{
			auto pointer = m_entity_manager.entity(worldEntity);
			if (pointer) {
				pointer->onUnloaded(this);
				free(pointer);
			}
			else ND_WARN("Unloading entity that has no instance");

			m_entity_manager.setLoaded(worldEntity, false);

			m_entity_array.erase(m_entity_array.begin() + i);
			return;
		}
	}
}

void World::killEntity(EntityID id)
{
	if (m_entity_manager.isLoaded(id))
		unloadEntity(id);
	else
		ND_WARN("Killing entity thats not loaded! {}", id);

	m_entity_manager.killEntity(id);
}

bool World::saveWorld()
{
	//world metadata
	auto session = WorldIO::Session(m_file_path, true);
	session.saveWorldMetadata(&m_info);
	session.close();

	m_nbt_saver.beginSession();

	//entity manager
	m_nbt_saver.setWriteChunkID(DYNAMIC_ID_ENITY_MANAGER);
	m_entity_manager.serialize(&m_nbt_saver);
	m_nbt_saver.flushWrite();

	//world nbt
	m_nbt_saver.setWriteChunkID(DYNAMIC_ID_WORLD_NBT);
	m_world_nbt.serialize(&m_nbt_saver);
	m_nbt_saver.flushWrite();

	m_nbt_saver.saveVTable();
	m_nbt_saver.endSession();

	return true;

}

bool World::loadWorld()
{
	if (!std::filesystem::exists(m_file_path))
		return false;

	//load worldinfo
	auto session = WorldIO::Session(m_file_path, true);
	if (!session.loadWorldMetadata(&m_info)) {
		ND_WARN("World file: {} is corrupted", m_file_path);
		return false;
	}
	session.close();

	m_nbt_saver.init();
	m_nbt_saver.beginSession();

	//load entitymanager
	if(m_nbt_saver.setReadChunkID(DYNAMIC_ID_ENITY_MANAGER))
		m_entity_manager.deserialize(&m_nbt_saver);
	else ND_WARN("World file: {}.entity is corrupted", m_file_path);

	//load worldnbt
	if(m_nbt_saver.setReadChunkID(DYNAMIC_ID_WORLD_NBT))
		m_world_nbt.deserialize(&m_nbt_saver);
	else ND_WARN("World file: {}.entity is corrupted (cannot read worldnbt)", m_file_path);

	m_nbt_saver.endSession();
	return true;
}

bool World::genWorld()
{
	auto session = WorldIO::Session(m_file_path, true,true);
	session.genWorldFile(&m_info);
	session.close();

	m_nbt_saver.clearEverything();
	m_nbt_saver.init();
	return true;

}


void World::loadEntity(WorldEntity* pEntity)
{
	auto id = pEntity->getID();
	//ASSERT(m_entity_manager.isAlive(id), "Loading dead entity, wtf..?");
	m_entity_manager.setLoaded(id, true);
	m_entity_manager.setEntityPointer(id, pEntity);
	m_entity_array.push_back(id);
	pEntity->onLoaded(this);
}

void World::setBlock(int x, int y, BlockStruct& blok)
{
#ifdef ND_DEBUG
	if (x < 0 || y < 0)
	{
		ND_WARN("Set block with invalid position");
		return;
	}
	if (x >= getInfo().chunk_width * WORLD_CHUNK_SIZE || y >= getInfo().chunk_height * WORLD_CHUNK_SIZE)
	{
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
	auto& block = c->block(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
	memcpy(blok.wall_id, block.wall_id, sizeof(blok.wall_id));
	memcpy(blok.wall_corner, block.wall_corner, sizeof(blok.wall_corner));

	c->setBlock(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1), blok);
	if (!m_edit_buffer_enable)
	{
		BlockRegistry::get().getBlock(blok.block_id).onNeighbourBlockChange(this, x, y);
		onBlocksChange(x, y);

		loadLightResources(x, y);
		m_light_calc.assignComputeChange(x, y); //refresh light around the block
	}
	else
		m_edit_buffer.emplace(x, y);
	c->markDirty(true); //always mark renderefresh
}
