#include "ndpch.h"
#include <utility>
#include "block/Block.h"
#include "WorldIO.h"
#include "block/BlockRegistry.h"
#include "biome/BiomeRegistry.h"
#include "App.h"
#include "entity/EntityRegistry.h"
#include <filesystem>
#include "entity/entities.h"
#include "FileChunkProvider.h"

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
		//m_chunks.at(i).m_loaded = false; redundant
	}
	memset(m_chunks.data(), 0, m_chunks.size() * sizeof(Chunk));
	ND_INFO("Made world instance");
}

World::World(std::string file_path, const WorldInfo& info)
	: m_light_calc(this),
	  m_threaded_gen(this),
	  m_job_pool(200),
	  m_info(info),
	  m_file_path(file_path),
	  m_nbt_saver(file_path + ".entity")
{
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

	//we need a buffer here cause we cant modify array during iteration
	if (m_entity_array_buff.size() != m_entity_array.size())
		m_entity_array_buff.resize(m_entity_array.size()); //make sure there is enough space in buff
	memcpy(m_entity_array_buff.data(), m_entity_array.data(), m_entity_array.size() * sizeof(EntityID));

	for (auto id : m_entity_array_buff)
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
	auto index = getChunkUnaccessibleIndex(id);
	if (index == -1)
		return -1;
	return m_chunk_headers[index].isAccessible() ? index : -1;
}

int World::getChunkUnaccessibleIndex(int id) const
{
	auto t = m_local_offset_header_map.find(id);
	if (t != m_local_offset_header_map.end())
		return t->second;
	return -1;
}

void World::loadChunk(int x, int y)
{
	std::set<int> s;
	s.insert(half_int(x, y));
	loadChunksAndGen(s);
}

constexpr int maskUp = BIT(0);
constexpr int maskDown = BIT(1);
constexpr int maskLeft = BIT(2);
constexpr int maskRight = BIT(3);
constexpr int maskGen = BIT(4);
constexpr int maskFreshlyOnlyLoaded = BIT(5); //chunk was only loaded by thread, no gen neccessary


void World::loadChunksAndGen(std::set<int>& toLoadChunks)
{
	//auto t = TimerStaper("loadAndGen took: ");
	// HOW TO LOAD AND GEN CHUNKS
	// 1.  determine which chunks need generation and which only load
	// 2.  add all those chunks and chunks around them to ToUpdateList
	// 3.  check if those chunks are valid and if chunks around don't need to be generated
	// 4.  lock all chunks to prevent unload,(mark chunks to BEING_LOADED if neccessary)
	// 5.  assignJob to ChunkProvider
	// 6.  when done, mark all chunks as loaded in header
	// 7.  call genChunks 
	// 8.  assign generation
	// 9.  when done finally register chunk to local_offset_map
	// 10. update all neccessary borders on main thread

	defaultable_map<int, int, 0> toupdateChunks;
	toupdateChunks.reserve(toupdateChunks.size() * 4);

	for (int chunkId : toLoadChunks)
	{
		int x = half_int::X(chunkId);
		int y = half_int::Y(chunkId);

		ASSERT(isChunkValid(x, y), "Invalid chunk pos");
		auto state = getChunkState(chunkId);
		if (!isChunkGenerated(chunkId) && state == UNLOADED)
		{
			//add chunks that need to be loaded and or refreshed

			toupdateChunks[half_int(x, y)] = maskRight | maskLeft | maskUp | maskDown | maskGen;
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
		else //add all other chunks that need to be loaded
			toupdateChunks[chunkId] |= 0;
	}

	std::vector<half_int> toRemove;
	toRemove.reserve(toupdateChunks.size());


	//now we need to load everything and lock it
	JobAssignmentP assignment = nullptr;
	int lastFreeChunk = 0;

	auto p = dynamic_cast<ThreadedChunkProvider*>(m_chunk_provider);
	p->beginBuffering();
	for (auto c : toupdateChunks)
	{
		auto chunkId = c.first;
		auto flags = c.second;
		if (!isChunkValid(chunkId))
		{
			toRemove.emplace_back(chunkId);
			continue;
		}
		int x = half_int::X(chunkId);
		int y = half_int::Y(chunkId);

		auto state = getChunkState(chunkId);

		//cannot stop unloading proccess
		if (state == BEING_UNLOADED)
		{
			toRemove.emplace_back(chunkId);
			continue;
		}
		//if chunks is already (being) loaded we just lock it to prevent unload 
		if (state == BEING_LOADED || state == BEING_GENERATED || state == GENERATED)
		{
			toupdateChunks[chunkId] = flags & (~maskGen); //remove togenflag
			m_chunk_headers[getChunkUnaccessibleIndex(chunkId)].getJob()->assign();
			continue;
		}
		//check if chunk to be loaded has not been generated yet, so we don't want to generate it
		if (!isChunkGenerated(chunkId) && (flags & maskGen) == 0)
		{
			toRemove.emplace_back(chunkId);
			continue;
		}

		int chunkOffset = getNextFreeChunkIndex(lastFreeChunk);
		if (chunkOffset == -1)
		{
			ND_WARN("Cannot load chunk, buffer is full. increment CHUNK_BUFFER_LENGTH");
			toRemove.emplace_back(chunkId);
			continue;
		}
		lastFreeChunk = chunkOffset + 1;

		auto& header = m_chunk_headers[chunkOffset];
		header = ChunkHeader(chunkId);
		header.getJob()->assign();
		header.setAccessible(false);
		header.setState(BEING_LOADED);

		m_local_offset_header_map[chunkId] = chunkOffset;
		//ND_INFO("chunkload {} {}", chunkId, chunkOffset);

		if (assignment == nullptr)
			assignment = m_job_pool.allocate();
		assignment->assign();
		if (!(maskGen & c.second))
			toupdateChunks[chunkId] |= maskFreshlyOnlyLoaded;
		m_chunk_provider->assignChunkLoad(assignment, &m_chunks[chunkOffset], getChunkSaveOffset(x, y));
	}
	p->flushBuffering();
	for (int id : toRemove) //remove invalid chunks from list
		toupdateChunks.erase(toupdateChunks.find(id));

	//wait for everything to load, then call genChunks
	//todo use std::move to move array
	auto afterLoad = [assignment, this, toupdateChunks{std::move(toupdateChunks)}]() mutable -> bool
	{
		if (assignment != nullptr)
			if (!assignment->isDone())
				return false;
		if (assignment)
			this->m_job_pool.deallocate(assignment);
		genChunks(toupdateChunks);
		return true;
	};
	App::get().getScheduler().runTaskTimer(afterLoad, 1);
}

void World::genChunks(defaultable_map<int, int, 0>& toUpdateChunks)
{
	//auto t = TimerStaper("genchunks took: ");
	JobAssignment* assignment = nullptr;
	for (auto c : toUpdateChunks)
	{
		auto chunkId = c.first;
		int offset = getChunkUnaccessibleIndex(chunkId);
		ASSERT(offset != -1, "This should not happen");
		
		if (getChunkState(chunkId) == BEING_LOADED)
		{
			setChunkState(chunkId, GENERATED); //all needed chunks are loaded
			m_chunks[offset].markDirty(true); //all loaded chunks need to be rendered
		}

		if (!(c.second & maskGen))
			continue;


		auto& chunk = m_chunks[offset];
		chunk.m_x = half_int::X(chunkId);
		chunk.m_y = half_int::Y(chunkId);
		setChunkState(chunkId, BEING_GENERATED);

		if (assignment == nullptr)
			assignment = m_job_pool.allocate();
		assignment->assign();
		m_threaded_gen.assignChunkGen(assignment, &chunk);
	}
	auto afterGen = [assignment, this, toUpdateChunks{std::move(toUpdateChunks)}]() mutable -> bool
	{
		if (assignment != nullptr)
			if (!assignment->isDone())
				return false;
		if (assignment)
			this->m_job_pool.deallocate(assignment);

		std::vector<ChunkID> chunkEntitiesToLoad;
		for (auto pair : toUpdateChunks)
		{
			half_int chunkID = pair.first;
			int offset = getChunkUnaccessibleIndex(chunkID);

			//unlock all used chunks
			//make free access for setBlock() etc..
			auto& header = m_chunk_headers[offset];
			header.getJob()->markDone();
			//mark gen chunks as generated
			if (pair.second & maskGen)
			{
				m_is_chunk_gen_map.set(getChunkSaveOffset(chunkID), true);
				header.setState(GENERATED);
			}
			header.setAccessible(true);

			if (pair.second & maskFreshlyOnlyLoaded)
				chunkEntitiesToLoad.push_back(pair.first);
			/*if (pair.second & maskGen)
			{
				m_chunks[offset].getLightJob().assign();
				m_light_calc.assignComputeChunk(chunkID.x, chunkID.y);
			}*/
		}

		//afterEntityLoad mutable resources
		int mutableState = 0;
		JobAssignmentP entityLoadJob = nullptr;
		WorldEntity*** arrayOfEntityArrayPointers = nullptr;
		int* arrayOfEntityArraySizes = nullptr;

		//entities
		if (!chunkEntitiesToLoad.empty())
		{
			auto afterEntityLoad = [this,
					entityLoadJob,
					chunkEntitiesToLoad,
					mutableState,
					arrayOfEntityArrayPointers,
					arrayOfEntityArraySizes]() mutable -> bool
			{
				if (mutableState == 0)
				{
					mutableState = 1;
					arrayOfEntityArrayPointers = new WorldEntity**[chunkEntitiesToLoad.size()];
					arrayOfEntityArraySizes = new int[chunkEntitiesToLoad.size()];
					entityLoadJob = m_job_pool.allocate();
					for (int i = 0; i < chunkEntitiesToLoad.size(); ++i)
					{
						ChunkID id = chunkEntitiesToLoad[i];
						entityLoadJob->assign();
						m_chunk_provider->assignEntityLoad(entityLoadJob, id,
						                                   &arrayOfEntityArrayPointers[i],
						                                   &arrayOfEntityArraySizes[i]);
					}
				}
				else
				{
					if (entityLoadJob->isDone())
					{
						m_job_pool.deallocate(entityLoadJob);
						for (int i = 0; i < chunkEntitiesToLoad.size(); ++i)
						{
							int entityArraySize = arrayOfEntityArraySizes[i];
							auto entityArray = arrayOfEntityArrayPointers[i];
							for (int i = 0; i < entityArraySize; ++i)
							{
								ASSERT(entityArray[i] != nullptr, "Shi");
								loadEntity(entityArray[i]);
							}
							if (entityArraySize)
								delete[] entityArray;
						}
						delete[] arrayOfEntityArrayPointers;
						delete[] arrayOfEntityArraySizes;

						return true;
					}
				}
				return false;
			};

			afterEntityLoad(); //assign the entity load job
			App::get().getScheduler().runTaskTimer(afterEntityLoad, 1);
		}

		updateBounds(toUpdateChunks);
		return true;
	};
	if (assignment == nullptr)
		afterGen();
	else
		App::get().getScheduler().runTaskTimer(afterGen, 1);

	/*//load section============================================================================
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

	App::get().getScheduler().runTaskTimer(
		[genIndex, upIndex, this, toGenChunks, toupdateChunks]() mutable -> bool
		{
			taskActive = true;
			int numberOfGens = 0;
			auto genIt = toGenChunks.begin();
			std::advance(genIt, genIndex);
			while (genIt != toGenChunks.end())
			{
				Chunk* c = &m_chunks[getChunkIndex(*genIt)];

				this->m_gen.genLayer0(this->m_info.seed, this, *c);

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
				for (int i = 0; i < 4; ++i)
				{
					auto pC = getLoadedChunkPointerMutable(resources[i].first, resources[i].second);
					if (pC)
						pC->lightLock();
				}

				m_light_calc.assignComputeChunkBorders(c.getCX(), c.getCY());
				if (mask != 0) //check if chunk is not only for readonly purposes
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
		}, 1);*/
}

void World::updateBounds(defaultable_map<int, int, 0>& toUpdateChunks)
{
	//auto t = TimerStaper("bounds took: ");

	//assign bound update
	JobAssignmentP boundUpdateJob = nullptr;
	std::vector<ChunkPack> resources;
	for (auto pair : toUpdateChunks)
	{
		//update chunk bounds if necessary
		half_int chunkID = pair.first;
		if (pair.second == 0)
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
			boundUpdateJob = m_job_pool.allocate();
		boundUpdateJob->assign();
		m_threaded_gen.assignChunkBoundUpdate(boundUpdateJob, resource, pair.second);
	}
	//lock all chunks which are boun updated and souroundings
	for (auto& pack : resources)
		for (auto chunkP : pack)
			if (chunkP)
				m_chunk_headers[getChunkIndex(chunkP->chunkID())].getJob()->assign();
	if (boundUpdateJob)
		App::get().getScheduler().runTaskTimer(
			[boundUpdateJob, this, resources]() mutable -> bool
			{
				if (!boundUpdateJob->isDone())
					return false;
				this->m_job_pool.deallocate(boundUpdateJob);

				//unlock all chunks which are boun updated and souroundings
				for (auto& pack : resources)
					for (auto chunkP : pack)
						if (chunkP)
						{
							m_chunk_headers[getChunkIndex(chunkP->chunkID())].getJob()->markDone();
							chunkP->markDirty(true); //finally shall we render that guy
						}
				m_has_chunk_changed = true; //notify worldrender to gather chunks to render
				return true;
			});
	else m_has_chunk_changed = true; //notify worldrender to gather chunks to render
}

void World::unloadChunks(std::set<int>& chunk_ids)
{
	JobAssignmentP assignment = nullptr;
	std::vector<EntityID> toUnload;
	std::vector<int> chunksToUnload;


	for (half_int chunkId : chunk_ids)
	{
		int chunkOffset = getChunkIndex(chunkId);
		if (chunkOffset == -1)
			continue;
		auto& header = m_chunk_headers[chunkOffset];
		if (!header.getJob()->isDone())
			continue;

		auto& chunk = m_chunks[chunkOffset];

		//todo maybe mark chunks as markedDead which would unload it after it is not locked
		//	if (!chunk.getLightJob().isDone())
		//	continue;

		header.setAccessible(false);
		chunk.last_save_time = getWorldTicks();
		if (assignment == nullptr)
			assignment = m_job_pool.allocate();
		assignment->assign();
		m_chunk_provider->assignChunkSave(assignment, &chunk, getChunkSaveOffset(chunkId));
		chunksToUnload.push_back(chunkId);
		setChunkState(chunkId, BEING_UNLOADED);
	}
	if (chunksToUnload.empty())
		return;

	WorldEntity*** arrayOfEntityArrayPointers = new WorldEntity**[chunksToUnload.size()];
	int* arrayOfEntityArraySizes = new int[chunksToUnload.size()];

	for (int chunkIdx = 0; chunkIdx < chunksToUnload.size(); ++chunkIdx)
	{
		ChunkID chunkId = chunksToUnload[chunkIdx];
		auto& chunk = m_chunks[getChunkUnaccessibleIndex(chunkId)];
		auto rect = chunk.getChunkRectangle();

		std::vector<WorldEntity*> entities;

		for (int entityIdx = m_entity_array.size()-1; entityIdx >=0 ; --entityIdx)
		{
			EntityID id = m_entity_array[entityIdx];
			auto pointer = m_entity_manager.entity(id);
			ASSERT(pointer, "world array contains unloaded entities");
			if (rect.containsPoint(pointer->getPosition()))
			{
				bool temp = pointer->hasFlag(EFLAG_TEMPORARY);
				unloadEntityNoDestruction(pointer, temp);
				m_entity_array.erase(m_entity_array.begin()+entityIdx);
				if(!temp)//save if not temporary
					entities.push_back(pointer);
				else 
					free(pointer);
			}
		}
		arrayOfEntityArraySizes[chunkIdx] = entities.size();
		arrayOfEntityArrayPointers[chunkIdx] = nullptr;
		if (!entities.empty())
		{
			arrayOfEntityArrayPointers[chunkIdx] = new WorldEntity*[entities.size()];
			memcpy(arrayOfEntityArrayPointers[chunkIdx], entities.data(), entities.size() * sizeof(WorldEntity*));
			assignment->assign();
			m_chunk_provider->assignEntitySave(assignment, chunkId, arrayOfEntityArrayPointers[chunkIdx],
			                                   arrayOfEntityArraySizes[chunkIdx]);
		}
	}

	App::get().getScheduler().runTaskTimer(
		[assignment, this, arrayOfEntityArrayPointers, arrayOfEntityArraySizes, chunksToUnload{
			std::move(chunksToUnload)
		}]() mutable -> bool
		{
			if (!assignment->isDone())
				return false;
			this->m_job_pool.deallocate(assignment);

			for (int chunkIdx = 0; chunkIdx < chunksToUnload.size(); ++chunkIdx)
			{
				auto id = chunksToUnload[chunkIdx];
				auto offset = getChunkUnaccessibleIndex(id);
				m_local_offset_header_map.erase(id);
				m_chunk_headers[offset] = ChunkHeader(); //reset head

				//free entitypointers
				for (int entityIdx = 0; entityIdx < arrayOfEntityArraySizes[chunkIdx]; ++entityIdx)
					free(arrayOfEntityArrayPointers[chunkIdx][entityIdx]);
				if (arrayOfEntityArraySizes[chunkIdx])
					delete[] arrayOfEntityArrayPointers[chunkIdx];

				//ND_INFO("chunkunlo {} {}", id, offset);
			}
			delete[] arrayOfEntityArraySizes;
			delete[] arrayOfEntityArrayPointers;

			return true;
		}, 1);

	/*WorldIO::Session stream = WorldIO::Session(m_file_path, true);
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
		m_local_offset_map.erase(chunkId); //is complexity bad? nope
		c.setLoaded(false);
		c.last_save_time = getWorldTicks();
		stream.saveChunk(&c, getChunkSaveOffset(c.chunkID()));

		int toUnloadRealSize = 0;
		for (EntityID id : m_entity_array)
		{
			auto e = m_entity_manager.entityDangerous(id);
			ASSERT(m_entity_manager.isLoaded(id) && m_entity_manager.isAlive(id), "Shit");
			if (rect.containsPoint(e->getPosition()))
			{
				toUnload.push_back(id);
				if (!e->hasFlag(EFLAG_TEMPORARY))
					toUnloadRealSize++;
			}
		}
		for (auto& pair : m_tile_entity_map)
		{
			auto e = m_entity_manager.entityDangerous(pair.second);
			ASSERT(m_entity_manager.isLoaded(pair.second) && m_entity_manager.isAlive(pair.second), "Shit");
			if (rect.containsPoint(e->getPosition()))
			{
				toUnload.push_back(pair.second);
				if (!e->hasFlag(EFLAG_TEMPORARY))
					toUnloadRealSize++;
			}
		}
		m_nbt_saver.setWriteChunkID(chunkId);
		if (toUnloadRealSize)
			m_nbt_saver.write(toUnloadRealSize); //if chunk is blank no need to write anything
		NBT nbt;
		for (auto entity : toUnload)
		{
			auto entiP = m_entity_manager.entity(entity);
			if (entiP->hasFlag(EFLAG_TEMPORARY))
			{
				killEntity(entiP->getID()); //those which temporary are, on chunk-unload shall be killed
			}
			else
			{
				entiP->save(nbt);
				nbt.serialize(&m_nbt_saver);
				nbt.clear();
				unloadEntity(entity);
			}
		}
		m_nbt_saver.flushWrite();
		toUnload.clear();
	}
	m_nbt_saver.endSession();*/
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
	auto resources = LightCalculator::computeQuadroSquare(x, y);
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
		ND_WARN("INvalid world location");
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
		//loadLightResources(pair.first, pair.second);
		//m_light_calc.assignComputeChange(pair.first, pair.second); //refresh light around the block
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

		//loadLightResources(x, y);
		//m_light_calc.assignComputeChange(x, y); //refresh light around the block
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
	if (isBlockValid(x, y + 1) && BlockRegistry::get()
	                              .getBlock(getBlockM(x, y + 1)->block_id).onNeighbourBlockChange(*this, x, y + 1))
	{
		getChunkM(World::toChunkCoord(x), World::toChunkCoord(y + 1))->markDirty(true);
		onBlocksChange(x, y + 1, deep);
	}

	if (isBlockValid(x, y - 1) && BlockRegistry::get()
	                              .getBlock(getBlockM(x, y - 1)->block_id).onNeighbourBlockChange(*this, x, y - 1))
	{
		getChunkM(World::toChunkCoord(x), World::toChunkCoord(y - 1))->markDirty(true);
		onBlocksChange(x, y - 1, deep);
	}

	if (isBlockValid(x + 1, y) && BlockRegistry::get()
	                              .getBlock(getBlockM(x + 1, y)->block_id).onNeighbourBlockChange(*this, x + 1, y))
	{
		getChunkM(World::toChunkCoord(x + 1), World::toChunkCoord(y))->markDirty(true);
		onBlocksChange(x + 1, y, deep);
	}

	if (isBlockValid(x - 1, y) && BlockRegistry::get()
	                              .getBlock(getBlockM(x - 1, y)->block_id).onNeighbourBlockChange(*this, x - 1, y))
	{
		getChunkM(World::toChunkCoord(x - 1), World::toChunkCoord(y))->markDirty(true);
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
	//loadLightResources(x, y);
	//m_light_calc.assignComputeChange(x, y);

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
	if (dynamic_cast<EntityPlayer*>(entity))
		ND_INFO("UNLO: {}", id);

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
	free(pointer);
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
	free(pointer);
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
	ASSERT(!m_entity_manager.isLoaded(id), "loading of loaded entity");
	m_entity_manager.setLoaded(id, true);
	m_entity_manager.setEntityPointer(id, pEntity);
	if (dynamic_cast<EntityPlayer*>(pEntity))
		ND_INFO("LOAD: {}", id);
	if (dynamic_cast<TileEntity*>(pEntity))
	{
		m_tile_entity_map[Phys::Vecti(pEntity->getPosition().x, pEntity->getPosition().y).toInt64()] = pEntity->getID();
	}
	else
		m_entity_array.push_back(id);
	pEntity->onLoaded(*this);
}

//======================WORLD IO=========================

bool World::saveWorld()
{
	//world metadata
	auto session = WorldIO::Session(m_file_path, true);
	session.saveWorldMetadata(&m_info);
	session.saveGenBoolMap(&m_is_chunk_gen_map);
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
	if (!session.loadWorldMetadata(&m_info))
	{
		ND_WARN("World file: {} is corrupted", m_file_path);
		return false;
	}
	session.loadGenBoolMap(&m_is_chunk_gen_map);
	session.close();

	m_nbt_saver.init();
	m_nbt_saver.beginSession();

	//load entitymanager
	if (m_nbt_saver.setReadChunkID(DYNAMIC_ID_ENITY_MANAGER))
		m_entity_manager.deserialize(&m_nbt_saver);
	else ND_WARN("World file: {}.entity is corrupted", m_file_path);

	//load worldnbt
	if (m_nbt_saver.setReadChunkID(DYNAMIC_ID_WORLD_NBT))
		m_world_nbt.deserialize(&m_nbt_saver);
	else ND_WARN("World file: {}.entity is corrupted (cannot read worldnbt)", m_file_path);

	m_nbt_saver.endSession();
	return true;
}

bool World::genWorld()
{
	auto session = WorldIO::Session(m_file_path, true, true);
	session.genWorldFile(&m_info);
	session.close();

	m_nbt_saver.clearEverything();
	m_nbt_saver.init();
	return true;
}

//=========================PARTICLES=====================

void World::spawnParticle(ParticleID id, Phys::Vect pos, Phys::Vect speed, Phys::Vect acc, int life, float rotation)
{
	m_particle_manager->createParticle(id, pos, speed, acc, life, rotation);
}
