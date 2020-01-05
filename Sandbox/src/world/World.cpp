#include "ndpch.h"
#include <utility>
#include "block/Block.h"
#include "WorldIO.h"
#include "block/BlockRegistry.h"
#include "biome/BiomeRegistry.h"
#include "core/App.h"
#include "entity/EntityRegistry.h"
#include <filesystem>
#include "entity/entities.h"
#include "FileChunkProvider.h"
#include "memory/stack_allocator.h"
#include "entity/EntityAllocator.h"

#define CHUNK_NOT_EXIST -1
constexpr int CHUNK_BUFFER_LENGTH = 400; //5*4

#define WORLD_CHECK_VALID_POS(wx,wy)\
	if ((wx) < 0 || (wy) < 0||(wx) >= getInfo().chunk_width * WORLD_CHUNK_SIZE || (wy) >= getInfo().chunk_height * WORLD_CHUNK_SIZE){\
		ND_WARN("Set block with invalid position");\
		return;\
	}

//=================WORLD======================================

void World::init()
{
	m_chunks.reserve(CHUNK_BUFFER_LENGTH);
	m_chunk_headers.resize(CHUNK_BUFFER_LENGTH);
	for (int i = 0; i < CHUNK_BUFFER_LENGTH; i++)
	{
		m_chunks.emplace_back();
	}
	memset(m_chunks.data(), 0, m_chunks.size() * sizeof(Chunk));
	ND_INFO("Made world instance");
}

World::World(std::string file_path, const WorldInfo& info)
	: m_light_calc(this),
	  m_threaded_gen(this),
	  m_info(info),
	  m_file_path(file_path),
	  m_nbt_saver(file_path + ".entity")
{
	m_nbt_saver.initIfExist();
	auto provider = new FileChunkProvider(file_path);
	m_is_chunk_gen_map.resize(info.chunk_width * info.chunk_height);
	memset((void*)m_is_chunk_gen_map.getSource().data(), 0, m_is_chunk_gen_map.byteSize());
	m_threaded_gen.start();
	provider->start();
	m_chunk_provider = provider;
	init();
}

World::~World()
{
	delete m_chunk_provider;
}

static bool taskActive = false;
static std::queue<std::set<int>> loadQueue;


void World::onUpdate()
{
	tick();
	/*if (!taskActive && !loadQueue.empty())
	{
		taskActive = true;
		genChunks(loadQueue.front());
		loadQueue.pop();
	}*/
}

static int light_tick_delay = 0;
constexpr int MAX_LIGHT_TICK_DELAY = 60;

void World::tick()
{
	static float timeAdvance = 0;
	timeAdvance += m_time_speed;
	while (timeAdvance >= 1)
	{
		m_info.time++;
		timeAdvance--;
	}

	nd::temp_vector<EntityID> entityArrayBuff(m_entity_array.size());
	memcpy(entityArrayBuff.data(), m_entity_array.data(), m_entity_array.size() * sizeof(EntityID));

	for (auto id : entityArrayBuff)
	{
		WorldEntity* entity = m_entity_manager.entity(id);
		if (entity)
		{
			entity->update(*this);
			if (entity->isMarkedDead())
				killEntity(entity->getID());
		}
	}

	m_tile_entity_array_buff = std::unordered_map<int64_t, EntityID>(m_tile_entity_map); //lets just copy

	for (auto& pair : m_tile_entity_array_buff)
	{
		WorldEntity* entity = m_entity_manager.entity(pair.second);
		if (entity)
		{
			entity->update(*this);
			if (entity->isMarkedDead())
				killEntity(entity->getID());
		}
	}
	m_particle_manager->update();
}


constexpr float minLight = 0.2f;

constexpr float startRiseHour = 5.f;
constexpr float endRiseHour = 6.f;

constexpr float startSetHour = 18.f;
constexpr float endSetHour = 19.f;

static float smootherstep(float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

glm::vec4 World::getSkyLight()
{
	auto hour = getWorldTime().hour();

	//hour = 12;
	float f = 0;
	if (hour >= startRiseHour && hour < endRiseHour)
	{
		hour -= startRiseHour;
		hour /= (endRiseHour - startRiseHour);
		f = minLight + smootherstep(hour) * (1 - minLight);
	}
	else if (hour >= startSetHour && hour < endSetHour)
	{
		hour -= startSetHour;
		hour /= (endSetHour - startSetHour);
		f = minLight + (1 - smootherstep(hour)) * (1 - minLight);
	}
	else if (hour >= endRiseHour && hour < startSetHour)
		f = 1;
	else f = minLight;
	return glm::vec4(0, 0, 0, f);
}

//====================CHUNKS=========================

bool World::isChunkGenerated(int chunkId)
{
	return m_is_chunk_gen_map[getChunkSaveOffset(chunkId)];
}

const Chunk* World::getChunk(int cx, int cy) const
{
	int index = getChunkIndex(half_int(cx, cy));
	if (index == -1)
		return nullptr;
	return &m_chunks[index];
}

Chunk* World::getChunkM(int cx, int cy)
{
	return const_cast<Chunk*>(getChunk(cx, cy));
}

int World::getChunkIndex(int id) const
{
	auto index = getChunkInaccessibleIndex(id);
	if (index == -1)
		return -1;
	return m_chunk_headers[index].isAccessible() ? index : -1;
}

int World::getChunkInaccessibleIndex(int id) const
{
	auto t = m_local_offset_header_map.find(id);
	if (t != m_local_offset_header_map.end())
		return t->second;
	return -1;
}

void World::loadChunk(int x, int y)
{
	nd::temp_set<int> s;
	s.insert(half_int(x, y));
	loadChunksAndGen(s);
}

constexpr int maskUp = BIT(0);
constexpr int maskDown = BIT(1);
constexpr int maskLeft = BIT(2);
constexpr int maskRight = BIT(3);
constexpr int maskGen = BIT(4);
constexpr int maskFreshlyOnlyLoaded = BIT(5); //chunk was only loaded by thread, no gen neccessary
constexpr int maskAllSides = maskUp | maskDown | maskLeft | maskRight;
//chunk was only loaded by thread, no gen neccessary

static defaultable_map<ChunkID, bool, false> CHUNK_MAP;

// calls genchunks2 with promise:
//		all chunks in toLoadChunks	are valid
//									have their header and space
//									have GENERATED or BEING_LOADED state
//									maskGen which should be generated
//									maskFreshlyOnlyLoaded should load their entities

void World::loadChunksAndGen(nd::temp_set<int>& toLoadChunks)
{
	defaultable_map<int, int, 0> toupdateChunks;
	toupdateChunks.reserve(50);
	nd::temp_vector<int> toRemove;
	toRemove.reserve(50);

	for (auto chunkID : toLoadChunks)
	{
		int x = half_int::X(chunkID);
		int y = half_int::Y(chunkID);

		if (!isChunkValid(x, y))
			continue;

		if (getChunkState(chunkID) == BEING_UNLOADED)
			continue;

		if (isChunkGenerated(chunkID) || getChunkState(chunkID) != UNLOADED)
		{
			//only load is neccessary
			toupdateChunks[chunkID] |= 0;
		}
		else
		{
			toupdateChunks[chunkID] |= maskRight | maskLeft | maskUp | maskDown | maskGen;
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
	}
	//filter invalid chunks
	for (auto& pair : toupdateChunks)
	{
		auto chunkID = pair.first;
		auto flags = pair.second;

		//valid filter
		if (!isChunkValid(chunkID))
		{
			toRemove.push_back(chunkID);
			continue;
		}
		//being unloaded filter
		if (getChunkState(chunkID) == BEING_UNLOADED)
		{
			toRemove.push_back(chunkID);
			continue;
		}
		//unwanted generation filter
		if (!(flags & maskGen) && !isChunkGenerated(chunkID) && getChunkState(chunkID) == UNLOADED)
		{
			toRemove.push_back(chunkID);
			continue;
		}
	}

	for (auto to_remove : toRemove)
		toupdateChunks.erase(toupdateChunks.find(to_remove));

	JobAssignmentP loadJob = nullptr;
	int lastFreeChunk = 0;
	std::vector<ChunkID> chunksToWaitFor;
	for (auto& pair : toupdateChunks)
	{
		auto chunkID = pair.first;
		auto flags = pair.second;
		auto state = getChunkState(chunkID);

		//need to find space for chunk and load it if already generated
		if (state == UNLOADED)
		{
			int chunkOffset = getNextFreeChunkIndex(lastFreeChunk);
			if (chunkOffset == -1)
			{
				ND_WARN("Cannot load chunk, buffer is full. increment CHUNK_BUFFER_LENGTH");
				//toRemove.emplace_back(chunkID);
				continue;
			}
			lastFreeChunk = chunkOffset + 1;

			auto& header = m_chunk_headers[chunkOffset];
			header = ChunkHeader(chunkID);
			header.getJob()->assign();
			header.setAccessible(false);
			header.setState(BEING_LOADED);

			m_local_offset_header_map[chunkID] = chunkOffset;

			// assign load if chunk has been generated in the past
			if (!(flags & maskGen))
			{
				toupdateChunks[pair.first] |= maskFreshlyOnlyLoaded;
				if (loadJob == nullptr)
					loadJob = ND_SCHED.allocateJob();
				m_chunk_provider->assignChunkLoad(loadJob, &m_chunks[chunkOffset], getChunkSaveOffset(chunkID));
			}
		}
			//just lock the chunk to prevent unload
		else if (state == GENERATED)
		{
			auto& header = m_chunk_headers[getChunkInaccessibleIndex(chunkID)];
			header.getJob()->assign();
		}
			//we will need to wait for chunk_provider and chunk_gen to finish all prior work
			//lock the chunk to prevent unload
		else
		{
			chunksToWaitFor.push_back(chunkID);
			auto& header = m_chunk_headers[getChunkInaccessibleIndex(chunkID)];
			header.getJob()->assign();
		}
	}
	// need to wait for our loading jobs, and all prior jobs of workers (i.e. chunk_provider, threaded_gen)
	if (!chunksToWaitFor.empty())
	{
		JobAssignmentP loadFence = ND_SCHED.allocateJob();
		JobAssignmentP genFence = ND_SCHED.allocateJob();
		bool fencesDone = false;
		m_chunk_provider->assignWait(loadFence);
		m_threaded_gen.assignWait(genFence);
		auto waitForWorkFunc = [this, fencesDone, chunksToWaitFor, loadFence, genFence, loadJob, toupdateChunks
			]()mutable-> bool
		{
			if (!fencesDone && loadFence->isDone() && genFence->isDone() && (loadJob == nullptr || loadJob->isDone()))
			{
				ND_SCHED.deallocateJob(loadFence);
				ND_SCHED.deallocateJob(genFence);
				if (loadJob)
					ND_SCHED.deallocateJob(loadJob);
				fencesDone = true;
			}
			if (fencesDone)
			{
				for (ChunkID chunkID : chunksToWaitFor)
					if (getChunkState(chunkID) != GENERATED)
						return false;
				genChunks(toupdateChunks);
				return true;
			}

			return false;
		};
		ND_SCHED.runTaskTimer(waitForWorkFunc);
	}
		//need to wait only for our load job
	else if (loadJob)
	{
		ND_SCHED.callWhenDone([this, toupdateChunks]() mutable
			{
				genChunks(toupdateChunks);
			}, loadJob);
	}
		//no need to wait for any load
	else genChunks(toupdateChunks);
}

void World::genChunks(defaultable_map<int, int, 0>& toUpdateChunks)
{
	JobAssignmentP genJob = nullptr;
	std::vector<ChunkID> entitiesToLoad;
	for (auto& pair : toUpdateChunks)
	{
		auto chunkID = pair.first;
		auto flags = pair.second;
		ASSERT(getChunkState(chunkID) == BEING_LOADED || getChunkState(chunkID) == GENERATED, "shit");
		auto offset = getChunkInaccessibleIndex(chunkID);
		auto& header = m_chunk_headers[offset];
		if (flags & maskGen)
		{
			header.setState(BEING_GENERATED);
			if (genJob == nullptr)
				genJob = ND_SCHED.allocateJob();
			m_chunks[offset].m_x = half_int::X(chunkID);
			m_chunks[offset].m_y = half_int::Y(chunkID);
			m_threaded_gen.assignChunkGen(genJob, &m_chunks[offset]);
		}
		else if (flags & maskFreshlyOnlyLoaded)
			entitiesToLoad.push_back(chunkID);
	}
	if (genJob != nullptr)
	{
		auto afterGen = [this, toUpdateChunks, entitiesToLoad]() mutable
		{
			//before bounds, all chunks need to be accessible
			//all chunks are generated by now
			for (auto pair : toUpdateChunks)
			{
				auto chunkID = pair.first;
				m_is_chunk_gen_map.set(getChunkSaveOffset(chunkID), true);
				auto& header = m_chunk_headers[getChunkInaccessibleIndex(chunkID)];
				header.setAccessible(true);
			}

			auto job = updateBounds2(toUpdateChunks);
			auto afterBound = [this, toUpdateChunks, entitiesToLoad]()mutable
			{
				updateLight(toUpdateChunks);
				loadEntFinal(toUpdateChunks, entitiesToLoad);
			};
			ND_SCHED.callWhenDone(afterBound, job);
		};
		ND_SCHED.callWhenDone(afterGen, genJob);
	}
	else
	{
		//set all to accessible for entities to enjoy
		for (auto pair : toUpdateChunks)
		{
			auto chunkID = pair.first;
			auto& header = m_chunk_headers[getChunkInaccessibleIndex(chunkID)];
			header.setAccessible(true);
		}
		updateLight(toUpdateChunks);
		loadEntFinal(toUpdateChunks, entitiesToLoad);
	}
}

void World::updateLight(defaultable_map<int, int, 0>& toUpdateChunks)
{
	//compute generated chunks
	for (auto pair : toUpdateChunks)
	{
		half_int chunkID = pair.first;
		auto flags = pair.second;
		if (flags & maskGen)
		{
			getChunkM(chunkID)->getLightJob().assign();
			m_light_calc.assignComputeChunk(chunkID.x, chunkID.y);
		}
	}
	//update bounds around others
	for (auto pair : toUpdateChunks)
	{
		half_int chunkID = pair.first;
		auto flags = pair.second;
		if (flags & maskAllSides)
		{
			ChunkPack res(chunkID, {
				              nullptr, getChunkM(chunkID.x, chunkID.y + 1), nullptr,
				              getChunkM(chunkID.x - 1, chunkID.y), getChunkM(chunkID),
				              getChunkM(chunkID.x + 1, chunkID.y),
				              nullptr, getChunkM(chunkID.x, chunkID.y - 1), nullptr,
			              });
			res.assignLightJob();
			m_light_calc.assignComputeChunkBorders(chunkID.x, chunkID.y, res);
		}
	}
}

void World::loadEntFinal(defaultable_map<int, int, 0>& toUpdateChunks, std::vector<int>& chunkEntitiesToLoad)
{
	auto afterEntity = [this, toUpdateChunks]()mutable
	{
		for (auto pair : toUpdateChunks)
		{
			auto chunkID = pair.first;
			auto& header = m_chunk_headers[getChunkInaccessibleIndex(chunkID)];
			header.getJob()->markDone(); //unlock all chunks
			header.setState(GENERATED);
			m_chunks[getChunkInaccessibleIndex(chunkID)].markDirty(true);
		}
		m_has_chunk_changed = true;
	};
	if (chunkEntitiesToLoad.empty())
		afterEntity();
	else
	{
		auto entityLoadedFence = loadEntities2(chunkEntitiesToLoad);
		if (entityLoadedFence->isDone())
		{
			afterEntity();
			ND_SCHED.deallocateJob(entityLoadedFence);
		}
		else
			ND_SCHED.callWhenDone(afterEntity, entityLoadedFence);
	}
}

JobAssignmentP World::loadEntities2(std::vector<int>& chunkEntitiesToLoad)
{
	JobAssignmentP p = ND_SCHED.allocateJob();

	JobAssignmentP entityLoadedFence = ND_SCHED.allocateJob();
	entityLoadedFence->assign();

	WorldEntity*** entityPointerArrayChunkArray = new WorldEntity**[chunkEntitiesToLoad.size()];
	int* entityPointerArraySizeChunkArray = new int[chunkEntitiesToLoad.size()];

	for (int i = 0; i < chunkEntitiesToLoad.size(); ++i)
	{
		//lock before entity load
		//m_chunk_headers[getChunkInaccessibleIndex(chunkEntitiesToLoad[i])].getJob()->assign();

		m_chunk_provider->assignEntityLoad(p, chunkEntitiesToLoad[i],
		                                   &entityPointerArrayChunkArray[i],
		                                   &entityPointerArraySizeChunkArray[i]);
	}

	auto afterLoad = [this, chunkEntitiesToLoad, entityLoadedFence, size{chunkEntitiesToLoad.size()},
			entityPointerArrayChunkArray,
			entityPointerArraySizeChunkArray]()mutable
	{
		for (int i = 0; i < size; ++i)
		{
			auto ray = entityPointerArrayChunkArray[i];
			auto raySize = entityPointerArraySizeChunkArray[i];
			for (int ii = 0; ii < raySize; ++ii)
				loadEntity(ray[ii]);
			if (raySize)
				delete[] ray;
			//unlock after entity load
			//m_chunk_headers[getChunkInaccessibleIndex(chunkEntitiesToLoad[i])].getJob()->markDone();
		}
		delete[] entityPointerArrayChunkArray;
		delete[] entityPointerArraySizeChunkArray;
		entityLoadedFence->markDone();
	};
	ND_SCHED.callWhenDone(afterLoad, p);
	return entityLoadedFence;
}

JobAssignmentP World::updateBounds2(defaultable_map<int, int, 0>& toUpdateChunks)
{
	//assign bound update
	JobAssignmentP boundUpdateJob = nullptr;
	std::vector<ChunkPack> resources;
	for (auto pair : toUpdateChunks)
	{
		//update chunk bounds if necessary
		half_int chunkID = pair.first;
		auto flags = pair.second;

		if ((flags & (maskDown | maskUp | maskLeft | maskRight)) == 0)
			continue;

		auto up = half_int(chunkID.x, chunkID.y + 1);
		auto down = half_int(chunkID.x, chunkID.y - 1);
		auto right = half_int(chunkID.x + 1, chunkID.y);
		auto left = half_int(chunkID.x - 1, chunkID.y);

		ChunkPack resource(chunkID, {
			                   nullptr, getChunkM(down), nullptr,
			                   getChunkM(left), getChunkM(chunkID), getChunkM(right),
			                   nullptr, getChunkM(up), nullptr
		                   });
		resources.push_back(resource); //todo use emplace instead of push
		if (boundUpdateJob == nullptr)
			boundUpdateJob = ND_SCHED.allocateJob();
		m_threaded_gen.assignChunkBoundUpdate(boundUpdateJob, resource, flags);
	}
	//lock all chunks which are boun updated and souroundings
	for (auto& pack : resources)
		for (auto chunkP : pack)
			if (chunkP)
				m_chunk_headers[getChunkIndex(chunkP->chunkID())].getJob()->assign();

	JobAssignmentP p = ND_SCHED.allocateJob();
	if (boundUpdateJob)
	{
		p->assign();
		auto afterBoundUpdate = [this,p, resources]() mutable
		{
			//unlock all chunks which are boun updated and souroundings
			for (auto& pack : resources)
				for (auto chunkP : pack)
					if (chunkP)
					{
						m_chunk_headers[getChunkIndex(chunkP->chunkID())].getJob()->markDone();
						chunkP->markDirty(true); //finally shall we render that guy
					}
			m_has_chunk_changed = true; //notify worldrender to gather chunks to render
			p->markDone();
		};
		ND_SCHED.callWhenDone(afterBoundUpdate, boundUpdateJob);
	}
	return p;
}


void World::unloadChunks(nd::temp_set<int>& chunk_ids)
{
	JobAssignmentP assignment = nullptr;
	std::vector<int> chunksToUnload;
	chunksToUnload.reserve(chunk_ids.size());


	for (half_int chunkId : chunk_ids)
	{
		int chunkOffset = getChunkIndex(chunkId);
		if (chunkOffset == -1)
			continue;
		auto& header = m_chunk_headers[chunkOffset];
		if (header.getState() != GENERATED) //cannot unload on any other state
			continue;
		if (!header.getJob()->isDone())
			continue;

		auto& chunk = m_chunks[chunkOffset];


		//todo maybe mark chunks as markedDead which would unload it after it is not locked
		if (!chunk.getLightJob().isDone())
			continue;

		header.setAccessible(false);
		header.setState(BEING_UNLOADED);
		header.getJob()->assign();
		chunk.last_save_time = getWorldTicks();
		if (assignment == nullptr)
			assignment = ND_SCHED.allocateJob();
		m_chunk_provider->assignChunkSave(assignment, &chunk, getChunkSaveOffset(chunkId));
		chunksToUnload.push_back(chunkId);
	}
	if (chunksToUnload.empty())
	{
		return;
	}

	WorldEntity*** arrayOfEntityArrayPointers = new WorldEntity**[chunksToUnload.size()];
	int* arrayOfEntityArraySizes = new int[chunksToUnload.size()];

	for (int chunkIdx = 0; chunkIdx < chunksToUnload.size(); ++chunkIdx)
	{
		ChunkID chunkId = chunksToUnload[chunkIdx];
		auto& chunk = m_chunks[getChunkInaccessibleIndex(chunkId)];

		std::vector<WorldEntity*> entities;

		for (int entityIdx = m_entity_array.size() - 1; entityIdx >= 0; --entityIdx)
		{
			EntityID id = m_entity_array[entityIdx];
			auto pointer = m_entity_manager.entity(id);
			ASSERT(pointer, "world array contains unloaded entities");
			if (chunkId == half_int(pointer->getPosition().x / WORLD_CHUNK_SIZE,
			                        pointer->getPosition().y / WORLD_CHUNK_SIZE))
			{
				bool temp = pointer->hasFlag(EFLAG_TEMPORARY);
				unloadEntityNoDestruction(pointer, temp);
				m_entity_array.erase(m_entity_array.begin() + entityIdx);
				if (temp) //save if not temporary
					EntityAllocator::deallocate(pointer);
				else
					entities.push_back(pointer);
			}
		}

		for (auto it = m_tile_entity_map.begin(); it != m_tile_entity_map.end();)
		{
			auto& pair = *it;
			auto pos = (Phys::Vecti*)&pair.first;
			EntityID id = pair.second;
			auto pointer = m_entity_manager.entity(id);
			ASSERT(pointer, "world array contains unloaded entities");

			if (chunkId == half_int(pos->x / WORLD_CHUNK_SIZE,
			                        pos->y / WORLD_CHUNK_SIZE))
			{
				bool temp = pointer->hasFlag(EFLAG_TEMPORARY);
				unloadEntityNoDestruction(pointer, temp);
				it = m_tile_entity_map.erase(it);
				if (temp) //save if not temporary
					EntityAllocator::deallocate(pointer);
				else
					entities.push_back(pointer);
			}
			else
			{
				++it;
			}
		}
		arrayOfEntityArraySizes[chunkIdx] = (int)entities.size();
		arrayOfEntityArrayPointers[chunkIdx] = nullptr;
		if (!entities.empty())
		{
			arrayOfEntityArrayPointers[chunkIdx] = new WorldEntity*[entities.size()];
			memcpy(arrayOfEntityArrayPointers[chunkIdx], entities.data(), entities.size() * sizeof(WorldEntity*));
		}
		m_chunk_provider->assignEntitySave(assignment, chunkId, arrayOfEntityArrayPointers[chunkIdx],
		                                   arrayOfEntityArraySizes[chunkIdx]);
	}
	auto afterUnload = [this, arrayOfEntityArrayPointers, arrayOfEntityArraySizes,
			chunksToUnload]() mutable
	{
		for (int chunkIdx = 0; chunkIdx < chunksToUnload.size(); ++chunkIdx)
		{
			//reset head
			auto id = chunksToUnload[chunkIdx];
			//ND_TRACE("unload ent chunk {}", half_int::toString(id));

			auto offset = getChunkInaccessibleIndex(id);
			m_local_offset_header_map.erase(id);
			m_chunk_headers[offset] = ChunkHeader();

			//free entitypointers
			for (int entityIdx = 0; entityIdx < arrayOfEntityArraySizes[chunkIdx]; ++entityIdx)
				EntityAllocator::deallocate(arrayOfEntityArrayPointers[chunkIdx][entityIdx]);
			if (arrayOfEntityArraySizes[chunkIdx])
				delete[] arrayOfEntityArrayPointers[chunkIdx];

			//ND_INFO("chunkunlo {} {}", id, offset);
		}
		delete[] arrayOfEntityArraySizes;
		delete[] arrayOfEntityArrayPointers;
	};
	ND_SCHED.callWhenDone(afterUnload, assignment);
}

void World::updateChunkBounds(BlockAccess& world, int cx, int cy, int bitBounds)
{
	auto& c = *world.getChunkM(cx, cy);
	int wx = c.m_x * WORLD_CHUNK_SIZE;
	int wy = c.m_y * WORLD_CHUNK_SIZE;
	if ((bitBounds & maskUp) != 0)
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			auto& block = c.block(x, WORLD_CHUNK_SIZE - 1);
			auto worldx = wx + x;
			auto worldy = wy + WORLD_CHUNK_SIZE - 1;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(world, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(world, worldx, worldy);
		}
	if ((bitBounds & maskDown) != 0)
		for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
		{
			auto& block = c.block(x, 0);
			auto worldx = wx + x;
			auto worldy = wy;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(world, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(world, worldx, worldy);
		}
	if ((bitBounds & maskLeft) != 0)
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.block(0, y);
			auto worldx = wx;
			auto worldy = wy + y;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(world, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(world, worldx, worldy);
		}
	if ((bitBounds & maskRight) != 0)
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.block(WORLD_CHUNK_SIZE - 1, y);
			auto worldx = wx + WORLD_CHUNK_SIZE - 1;
			auto worldy = wy + y;
			BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(world, worldx, worldy);
			if (block.isWallOccupied())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(world, worldx, worldy);
		}
}

void World::loadLightResources(int x, int y)
{
	//todo this needs rework
	auto resources = LightCalculator::createQuadroSquare(x, y);
	for (int i = 0; i < 4; ++i)
	{
		auto p = resources[i];
		if (isChunkValid(p.first, p.second))
		{
			//todo fix light
			/*if (!isChunkFullyLoaded(half_int(p.first, p.second)))
			{
				loadChunk(p.first, p.second);
			}
			//if(!getChunk(p.first,p.second).isLightLocked())
			//	ND_INFO("Lightlocked chunk: {},{} :", p.first, p.second, getChunk(p.first, p.second).isLightLocked());

			getChunkM(p.first, p.second)->getLightJob().assign();
		*/
		}
	}
}

int World::getNextFreeChunkIndex(int startSearchIndex)
{
	for (; startSearchIndex < m_chunk_headers.size(); ++startSearchIndex)
	{
		auto& h = m_chunk_headers[startSearchIndex];
		if (h.isFree())
			return startSearchIndex;
	}
	/*
	 *cannot resize arrays because of other workers possibly working on it right now
	 *
	m_chunks.reserve(m_chunks.size() + 1);
	m_chunks.emplace_back();
	m_chunk_headers.emplace_back();
	return m_chunks.size() - 1;*/
	//ASSERT(false, "Not enough space for next chunk. Raise the CHUNK_BUFFER_LENGTH");
	return -1;
}

//=========================BLOCKS==========================

const BlockStruct* World::getBlock(int x, int y) const
{
	if (!isValidLocation(x, y))
	{
		//ND_WARN("INvalid world location");
		return nullptr;
	}
	int index = getChunkIndex(half_int(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE));
	if (index != -1)
		return const_cast<BlockStruct*>(&m_chunks[index].block(x & (WORLD_CHUNK_SIZE - 1),
		                                                       y & (WORLD_CHUNK_SIZE - 1)));
	return nullptr;
}

const BlockStruct* World::getBlockOrAir(int x, int y) const
{
	auto t = getBlock(x, y);
	static BlockStruct b = 0;
	return t == nullptr ? &b : t;
}

BlockStruct* World::getBlockM(int x, int y)
{
	return const_cast<BlockStruct*>(getBlock(x, y));
}

void World::flushBlockSet()
{
	m_edit_buffer_enable = false;
	while (!m_edit_buffer.empty())
	{
		auto pair = m_edit_buffer.front();
		m_edit_buffer.pop();
		BlockRegistry::get().getBlock(getBlockM(pair.first, pair.second)->block_id).onNeighbourBlockChange(
			*this, pair.first, pair.second);
		onBlocksChange(pair.first, pair.second);

		//todo this is big bullshit we will need to separate it somehow to update in batches
		auto res = LightCalculator::createQuadroSquare(pair.first, pair.second);
		res.assignLightJob(*this);
		m_light_calc.assignComputeChange(pair.first, pair.second); //refresh light around the block
	}
}

void World::setBlockWithNotify(int x, int y, BlockStruct& newBlock)
{
	auto blok = getBlockM(x, y);
	if (blok == nullptr)
		return;

	auto& regOld = BlockRegistry::get().getBlock(blok->block_id);
	auto& regNew = BlockRegistry::get().getBlock(newBlock.block_id);


	memcpy(newBlock.wall_id, blok->wall_id, sizeof(newBlock.wall_id));
	memcpy(newBlock.wall_corner, blok->wall_corner, sizeof(newBlock.wall_corner));

	regOld.onBlockDestroyed(*this, nullptr, x, y, *blok);

	*blok = newBlock;

	if (!m_edit_buffer_enable)
	{
		regNew.onBlockPlaced(*this, nullptr, x, y, *blok);
		regNew.onNeighbourBlockChange(*this, x, y);
		onBlocksChange(x, y);

		auto res = LightCalculator::createQuadroSquare(x, y);
		res.assignLightJob(*this);
		m_light_calc.assignComputeChange(x, y); //refresh light around the block
	}
	else
		m_edit_buffer.emplace(x, y);

	getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE)->markDirty(true);
}

void World::setBlock(int x, int y, BlockStruct& newBlock)
{
	auto blok = getBlockM(x, y);
	if (blok == nullptr)
		return;

	auto& regOld = BlockRegistry::get().getBlock(blok->block_id);
	auto& regNew = BlockRegistry::get().getBlock(newBlock.block_id);


	memcpy(newBlock.wall_id, blok->wall_id, sizeof(newBlock.wall_id));
	memcpy(newBlock.wall_corner, blok->wall_corner, sizeof(newBlock.wall_corner));

	*blok = newBlock;
}

#define MAX_BLOCK_UPDATE_DEPTH 20

void World::onBlocksChange(int x, int y, int deep)
{
	++deep;
	if (deep >= MAX_BLOCK_UPDATE_DEPTH)
	{
		ND_WARN("Block update too deep! protecting stack");
		return;
	}

	auto p = getBlockM(x, y + 1);
	if (p && BlockRegistry::get().getBlock(p->block_id).onNeighbourBlockChange(*this, x, y + 1))
	{
		getChunkM(Chunk::getChunkIDFromWorldPos(x, y + 1))->markDirty(true);
		onBlocksChange(x, y + 1, deep);
	}
	p = getBlockM(x, y - 1);
	if (p && BlockRegistry::get().getBlock(p->block_id).onNeighbourBlockChange(*this, x, y - 1))
	{
		getChunkM(Chunk::getChunkIDFromWorldPos(x, y - 1))->markDirty(true);
		onBlocksChange(x, y - 1, deep);
	}
	p = getBlockM(x + 1, y);
	if (p && BlockRegistry::get().getBlock(p->block_id).onNeighbourBlockChange(*this, x + 1, y))
	{
		getChunkM(Chunk::getChunkIDFromWorldPos(x + 1, y))->markDirty(true);
		onBlocksChange(x + 1, y, deep);
	}
	p = getBlockM(x - 1, y);
	if (p && BlockRegistry::get().getBlock(p->block_id).onNeighbourBlockChange(*this, x - 1, y))
	{
		getChunkM(Chunk::getChunkIDFromWorldPos(x - 1, y))->markDirty(true);
		onBlocksChange(x - 1, y, deep);
	}
}

//=========================WALLS===========================

void World::setWall(int x, int y, int wall_id)
{
	auto blok = getBlockM(x, y);
	if (blok == nullptr)
		return;

	auto& regOld = BlockRegistry::get().getBlock(blok->block_id);

	if (!blok->isWallOccupied() && wall_id == 0) //cannot erase other blocks parts on this shared block
		return;
	blok->setWall(wall_id);
	onWallsChange(x, y, *blok);

	auto res = LightCalculator::createQuadroSquare(x, y);
	res.assignLightJob(*this);
	m_light_calc.assignComputeChange(x, y);

	getChunkM(x >> WORLD_CHUNK_BIT_SIZE, y >> WORLD_CHUNK_BIT_SIZE)->markDirty(true);
}

void World::onWallsChange(int xx, int yy, BlockStruct& blok)
{
	BlockRegistry::get().getWall(blok.wallID()).onNeighbourWallChange(*this, xx, yy);
	/*for (int x = -1; x < 2; ++x)
	{
		for (int y = -1; y < 2; ++y)
		{
			if (x == 0 && y == 0)
				continue;
			if (isValidBlock(x + xx, y + yy)) {
				BlockStruct& b = *getBlock(x + xx, y + yy);
				if (b.isWallOccupied())//i cannot call this method on some foreign wall pieces
					BlockRegistry::get().getWall(b.wallID()).onNeighbourWallChange(*this, x + xx, y + yy);
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
				auto& b = *getBlockM(x + xx, y + yy);
				if (b.isWallOccupied())
				{
					//i cannot call this method on some foreign wall pieces
					BlockRegistry::get().getWall(b.wallID()).onNeighbourWallChange(*this, x + xx, y + yy);
					auto& c = *getChunkM(toChunkCoord(x + xx), toChunkCoord(y + yy));
					c.markDirty(true); //mesh needs to be updated
				}
			}
		}
	}
}

//===================ENTITIES============================

EntityID World::spawnEntity(WorldEntity* pEntity)
{
	auto id = m_entity_manager.createEntity();

	pEntity->m_id = id;
	loadEntity(pEntity);
	pEntity->onSpawned(*this);
	return id;
}

void World::unloadEntityNoDestruction(WorldEntity* entity, bool isKilled)
{
	auto id = entity->getID();

	ASSERT(entity, "entity is null but is in m_entity_array");
	entity->onUnloaded(*this);
	if (isKilled)
		entity->onKilled(*this);
	m_entity_manager.setLoaded(id, false);

	if (isKilled)
		m_entity_manager.killEntity(id);
}

void World::unloadEntity(EntityID id, bool isKilled)
{
	auto pointer = m_entity_manager.entity(id);
	ASSERT(pointer, "cannot unload entity which does not have pointer");
	unloadEntityNoDestruction(pointer, isKilled);
	EntityAllocator::deallocate(pointer);
	for (int i = 0; i < m_entity_array.size(); ++i)
	{
		if (m_entity_array[i] == id)
		{
			m_entity_array.erase(m_entity_array.begin() + i);
			return;
		}
	}
	for (auto& pair : m_tile_entity_map)
	{
		if (pair.second == id)
		{
			m_tile_entity_map.erase(pair.first);
			return;
		}
	}
	ASSERT(false, "UNloading entity that was not in local world map");
}

void World::unloadTileEntity(EntityID id, bool isKilled)
{
	auto pointer = m_entity_manager.entity(id);
	ASSERT(pointer, "cannot unload entity which does not have pointer");
	unloadEntityNoDestruction(pointer, isKilled);
	EntityAllocator::deallocate(pointer);
	for (auto& pair : m_tile_entity_map)
	{
		if (pair.second == id)
		{
			m_tile_entity_map.erase(pair.first);
			return;
		}
	}
	ASSERT(false, "UNloading tile entity that was not in local world map");
}

void World::killEntity(EntityID id)
{
	if (m_entity_manager.isLoaded(id))
		unloadEntity(id, true);
	else
		ND_WARN("Killing entity thats not loaded! {}", id);
}

void World::killTileEntity(EntityID id)
{
	if (m_entity_manager.isLoaded(id))
		unloadTileEntity(id, true);
	else
		ND_WARN("Killing entity thats not loaded! {}", id);
}

void World::loadEntity(WorldEntity* pEntity)
{
	auto id = pEntity->getID();
	ASSERT(m_entity_manager.isAlive(id), "loading of dead entity");
	auto pointerToOld = m_entity_manager.entity(id);

	ASSERT(!m_entity_manager.isLoaded(id), "loading of loaded entity");
	m_entity_manager.setLoaded(id, true);
	m_entity_manager.setEntityPointer(id, pEntity);

	if (dynamic_cast<TileEntity*>(pEntity))
	{
		m_tile_entity_map[Phys::Vecti(pEntity->getPosition().x, pEntity->getPosition().y).toInt64()] = pEntity->getID();
	}
	else
		m_entity_array.push_back(id);
	pEntity->onLoaded(*this);
}

//======================WORLD IO=========================

JobAssignmentP World::saveWorld()
{
	JobAssignmentP job = App::get().getScheduler().allocateJob();

	//world metadata
	m_chunk_provider->assignWorldInfoSave(job, &m_info);
	//bool gen
	m_chunk_provider->assignBoolGenSave(job, &m_is_chunk_gen_map);
	//entity manager
	m_chunk_provider->assignSerialize(job, DYNAMIC_ID_ENTITY_MANAGER,
	                                  std::bind(&EntityManager::serialize, &m_entity_manager, std::placeholders::_1));
	//world nbt
	m_world_nbt.set<std::string>("teststring", "saving worldnbt");
	m_chunk_provider->assignNBTSave(job, DYNAMIC_ID_WORLD_NBT, &m_world_nbt);

	return job;
}

JobAssignmentP World::loadWorld()
{
	if (!std::filesystem::exists(m_file_path))
		return nullptr;

	JobAssignmentP job = App::get().getScheduler().allocateJob();

	//world metadata
	m_chunk_provider->assignWorldInfoLoad(job, &m_info);
	//bool gen
	m_chunk_provider->assignBoolGenLoad(job, &m_is_chunk_gen_map);
	//entity manager
	m_chunk_provider->assignDeserialize(job, DYNAMIC_ID_ENTITY_MANAGER,
	                                    std::bind(&EntityManager::deserialize, &m_entity_manager,
	                                              std::placeholders::_1));
	//world nbt
	m_chunk_provider->assignNBTLoad(job, DYNAMIC_ID_WORLD_NBT, &m_world_nbt);

	return job;
}

void World::genWorld()
{
	auto session = WorldIO::Session(m_file_path, true, true);
	session.genWorldFile(&m_info);
	session.close();

	m_is_chunk_gen_map.resize(m_info.chunk_width * m_info.chunk_height);

	//todo this should be on another thread as well
	m_nbt_saver.clearEverything();
	m_nbt_saver.init();
}

//=========================PARTICLES=====================

void World::spawnParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, int life,
                          float rotation)
{
	m_particle_manager->createParticle(id, pos, speed, acc, life, rotation);
}

void World::spawnBlockBreakParticles(int x, int y)
{
	auto blok = getBlock(x, y);
	if (blok == nullptr||blok->isAir())
		return;
	auto offset = BlockRegistry::get().getBlock(blok->block_id).getTextureOffset(x, y, *blok);

	

	glm::vec2 middle(0.5f,0);
	for (int xx = 0; xx < particleBlockDivision; ++xx)
		for (int yy = 0; yy < particleBlockDivision; ++yy) {
			float xxx = xx / (float)particleBlockDivision;
			float yyy = yy / (float)particleBlockDivision;
			float mutt = 0.5;
			auto velocity = (glm::vec2(xxx, yyy) - middle)*0.3f* mutt;
			velocity += glm::vec2(std::rand() % 2048 / 2048.f-0.5f, std::rand() % 2048 / 2048.f-0.5f) * 0.15f* mutt;
			m_particle_manager->createBlockParticle(offset, xx, yy, { x + xxx,y +yyy}, velocity, { -velocity.x/35* mutt,-0.011f* mutt }, 30, 0);
		}
}

nd::temp_vector<WorldEntity*> World::getEntitiesInRadius(const glm::vec2& pos, float radius)
{
	nd::temp_vector<WorldEntity*> out;
	out.reserve(10);
	for (auto entity : m_entity_array)
	{
		auto t = m_entity_manager.entity(entity);
		if (t == nullptr)
		{
			ERROR("Invalid entity id in m_entity_array");
		}
		if (glm::distance2(t->getPosition(), pos) < (radius * radius))
		{
			out.push_back(t);
		}
	}
	return out;
}

nd::temp_vector<WorldEntity*> World::getEntitiesAtLocation(const glm::vec2& pos)
{
	nd::temp_vector<WorldEntity*> out;
	out.reserve(5);
	for (auto entity : m_entity_array)
	{
		auto t = m_entity_manager.entity(entity);
		ASSERT(t, "Invalid entity id in m_entity_array");
		
		constexpr float maxDistance = 10;//from pos
		if (glm::distance2(t->getPosition(), pos) < (maxDistance * maxDistance))
		{
			auto eee = dynamic_cast<PhysEntity*>(t);
			if(eee && contains(eee->getCollisionBox(), pos - eee->getPosition()))
				out.push_back(t);
		}
	}
	return out;
}
