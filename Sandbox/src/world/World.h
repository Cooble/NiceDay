#pragma once
#include "ndpch.h"
#include "WorldIO.h"
#include "LightCalculator.h"
#include "gen/WorldGen.h"
#include "block/Block.h"
#include "entity/WorldEntity.h"
#include "world/WorldTime.h"
#include "particle/ParticleManager.h"
#include "ThreadedWorldGen.h"
#include "BlockAccess.h"
#include "memory/stack_allocator.h"
#include "core/NBT.h"


class IChunkProvider;




constexpr int DYNAMIC_ID_ENTITY_MANAGER = std::numeric_limits<int>::max() - 0;
constexpr int DYNAMIC_ID_WORLD_NBT = std::numeric_limits<int>::max() - 1;

const int WORLD_CHUNK_BIT_SIZE = 5;
const int WORLD_CHUNK_SIZE = 32;
const int WORLD_CHUNK_AREA = WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE;

static const int BITS_FOR_CHUNK_LOC = 16;

constexpr int CHUNK_DIRTY_FLAG = BIT(1); //in next update chunk graphics will be reloaded into chunkrenderer
constexpr int CHUNK_LOCKED_FLAG = BIT(2); //dont unload this chunk its being worked on by main thread

typedef int ChunkID;
class World;

class Chunk
{
private:
	int m_x, m_y;

	//posxy

	BlockStruct m_blocks[WORLD_CHUNK_AREA];
	uint8_t m_light_levels[WORLD_CHUNK_AREA];
	uint32_t m_flags = 0;
	int m_biome;

	//multithreading light
	JobAssignment m_light_job;

public:
	friend class World;
	friend class WorldIO::Session;
	friend class WorldGen;

	long long last_save_time;
	int getCX() const { return m_x; }
	int getCY() const { return m_y; }
	ChunkID chunkID() const { return half_int(m_x, m_y); }

	// cannot unload locked chunk
	bool isLocked() const
	{
		return (m_flags & CHUNK_LOCKED_FLAG) != 0 || (!m_light_job.isDone());
	}

	bool isDirty() const { return m_flags & CHUNK_DIRTY_FLAG; }
	bool isGenerated() const { return last_save_time != 0; } //worldgen has generated it
	void lock(bool lock)
	{
		if (lock) m_flags |= CHUNK_LOCKED_FLAG;
		else m_flags &= ~CHUNK_LOCKED_FLAG;
	}

	JobAssignment& getLightJob() { return m_light_job; }

	BlockStruct& block(int x, int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	const BlockStruct& block(int x, int y) const { return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x]; }

	uint8_t& lightLevel(int x, int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_light_levels[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	uint8_t lightLevel(int x, int y) const
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_light_levels[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	void markDirty(bool dirty)
	{
		if (dirty) m_flags |= CHUNK_DIRTY_FLAG;
		else m_flags &= ~CHUNK_DIRTY_FLAG;
	}

	int getBiome() const { return m_biome; }

	void setBiome(int biome_id) { m_biome = biome_id; }

	/*inline Phys::Rectangle getChunkRectangle() const
	{
		return Phys::Rectangle::createFromDimensions(m_x * WORLD_CHUNK_SIZE, m_y * WORLD_CHUNK_SIZE, WORLD_CHUNK_SIZE,
		                                             WORLD_CHUNK_SIZE);
	}*/

	static int getChunkIDFromWorldPos(int wx, int wy)
	{
		return half_int(wx >> WORLD_CHUNK_BIT_SIZE, wy >> WORLD_CHUNK_BIT_SIZE);
	}
};

struct WorldInfo
{
	long seed;
	int chunk_width, chunk_height;
	int terrain_level;
	long long time = 0;
	char name[100]{};
	EntityID player_id;
	ChunkID playerChunk;
};


class World : public BlockAccess
{
	friend class WorldIO::Session;
	friend class WorldGen;
public:
	enum ChunkState
	{
		BEING_LOADED,
		BEING_GENERATED,
		BEING_BOUNDED,
		GENERATED,
		BEING_UNLOADED,
		UNLOADED
	};

	class ChunkHeader
	{
		ChunkID m_chunkId = std::numeric_limits<int>::max();
		JobAssignment m_job;
		ChunkState m_state = UNLOADED;

		// whether you can call setBlock etc.
		// chunk might be generated at the moment so always check this
		bool m_is_accessible = false;

	public:
		ChunkHeader() = default;

		ChunkHeader(ChunkID id) : m_chunkId(id)
		{
		}
		ChunkHeader(const ChunkHeader& h) :
		m_chunkId(h.m_chunkId),
		m_job(h.m_job),
		m_is_accessible(h.m_is_accessible),
		m_state(h.m_state)
		{
		}
		
		ChunkID getChunkID() const { return m_chunkId; }
		JobAssignmentP getJob() { return &m_job; }
		const JobAssignment& getJobConst() const { return m_job; }

		bool isFree() const
		{
			return m_chunkId == std::numeric_limits<int>::max();
		}

		bool operator==(const ChunkHeader& h) { return m_chunkId == h.m_chunkId; }
		bool operator!=(const ChunkHeader& h) { return !(m_chunkId == h.m_chunkId); }
		void setState(ChunkState state) { m_state = state; }
		ChunkState getState() const { return m_state; }
		void setAccessible(bool a) { m_is_accessible = a; }
		bool isAccessible() const { return m_is_accessible; }
	};

	static int toChunkCoord(float x) { return toChunkCoord((int)x); }
	static int toChunkCoord(int x) { return x >> WORLD_CHUNK_BIT_SIZE; }
	float m_time_speed = 1;
	bool m_dayNightCycleEnable = true;
private:
	NDUtil::Bitset m_is_chunk_gen_map;
	LightCalculator m_light_calc;
	WorldGen m_gen;
	std::vector<Chunk> m_chunks;
	// will be set to true after first batch of chunks is generated (used to start light snapshots)
	bool m_first_chunks_generated=false;
	std::vector<ChunkHeader> m_chunk_headers;
	NBT m_world_nbt;
	IChunkProvider* m_chunk_provider;
	ThreadedWorldGen m_threaded_gen;

	bool m_has_chunk_changed = false;

	WorldInfo m_info;
	std::string m_file_path;

	//todo problem used by gen and main thread
	std::unordered_map<int, int> m_local_offset_header_map; //chunkID,offsetInBuffer
	BlockStruct m_air_block = 0;

	EntityManager m_entity_manager;

	// needs to be created outside world and set
	ParticleManager* m_particle_manager = nullptr;
	DynamicSaver m_nbt_saver;

	std::vector<EntityID> m_entity_array;
	std::unordered_map<int64_t, EntityID> m_tile_entity_map;
	std::vector<EntityID> m_entity_array_buff;
	std::unordered_map<int64_t, EntityID> m_tile_entity_array_buff;


	bool m_edit_buffer_enable = false;
	std::queue<std::pair<int, int>> m_edit_buffer; //location x,y for editted blocks

private:
	void init();
	void onBlocksChange(int x, int y, int deep = 0);
	void onWallsChange(int xx, int yy, BlockStruct& blok);
	int getNextFreeChunkIndex(int startSearchIndex = 0);
	void genChunks(defaultable_map<int, int, 0>& toUpdateChunks);
	void updateLight(defaultable_map<int, int, 0>& toUpdateChunks);

	void loadEntFinal(defaultable_map<int, int, 0>& toUpdateChunks, std::vector<int>& chunkEntitiesToLoad);

	JobAssignmentP loadEntities2(std::vector<int>& chunksToLoad);
	
	JobAssignmentP updateBounds2(defaultable_map<int, int, 0>& toUpdateChunks);
	void loadLightResources(int x, int y);

	ChunkState getChunkState(int chunkID)
	{
		auto it = m_local_offset_header_map.find(chunkID);
		if (it == m_local_offset_header_map.end())
			return ChunkState::UNLOADED;
		return m_chunk_headers[it->second].getState();
	}

	void setChunkState(int chunkID, ChunkState state)
	{
		auto it = m_local_offset_header_map.find(chunkID);
		ASSERT(it != m_local_offset_header_map.end(), "setting chunk state to chunk whose header is missing");
		m_chunk_headers[it->second].setState(state);
	}

	bool isValidLocation(float wx, float wy) const
	{
		return !((wx) < 0 || (wy) < 0 || (wx) >= getInfo().chunk_width * WORLD_CHUNK_SIZE || (wy) >= getInfo().chunk_height * WORLD_CHUNK_SIZE);
	}

public:
	World(std::string file_path, const WorldInfo& info);
	~World();

	LightCalculator& getLightCalculator() { return m_light_calc; }
	void onUpdate();
	void tick();


	//returns if a chunk was loaded or unloaded and resets
	bool hasChunkChanged()
	{
		bool out = m_has_chunk_changed;

		m_has_chunk_changed = false;
		return out;
	}
	bool areFirstChunksGenerated() { return m_first_chunks_generated; }

	bool isBlockValid(int x, int y) const
	{
		return x >= 0 && y >= 0 && x < getInfo().chunk_width * WORLD_CHUNK_SIZE && y < getInfo().chunk_height *
			WORLD_CHUNK_SIZE;
	}

	bool isChunkValid(int x, int y) const
	{
		return isChunkValid(half_int(x, y));
	}

	bool isChunkValid(half_int chunkid) const
	{
		return chunkid.x >= 0 && chunkid.x < getInfo().chunk_width && chunkid.y >= 0 && chunkid.y < getInfo().
			chunk_height;
	}

	glm::vec4 getSkyLight();
	bool isChunkGenerated(int chunkId);

	int getChunkSaveOffset(int id) const
	{
		return getChunkSaveOffset(half_int::X(id),half_int::Y(id));
	};
	int getChunkSaveOffset(int cx,int cy) const
	{
		return cx + cy*m_info.chunk_width;
	};
	//==============CHUNK METHODS========================================================================

	//return nullptr if chunk is not loaded or invalid coords 
	//(won't cause chunk load)
	const Chunk* getChunk(int x, int y) const;
	Chunk* getChunkM(int cx, int cy) override;
	Chunk* getChunkM(half_int chunkID) { return getChunkM(chunkID.x, chunkID.y); }

	// return index if chunk is in memory and accessible or -1
	int getChunkIndex(int id) const;
	// return index if chunk is in memory (array) or -1
	int getChunkInaccessibleIndex(int id) const;

	// assign chunk load and gen task and returns
	void loadChunk(int x, int y);

	// returns if chunk can be normally accessed (it is not in being loaded state)
	bool isChunkFullyLoaded(int id) const { return getChunkIndex(id) != -1; }

	void unloadChunks(nd::temp_set<int>& chunk_ids);
	static void updateChunkBounds(BlockAccess& world, int cx, int cy, int bitBounds);
	void loadChunksAndGen(nd::temp_set<int>& toLoadChunks);

	//==============BLOCK METHODS=======================================================================

	//return nullptr if block is not loaded or invalid coords 
	//(won't cause chunk load)
	const BlockStruct* getBlock(int x, int y) const;
	const Block& getBlockInstance(int x, int y) const;

	// never returns nullptr
	const BlockStruct* getBlockOrAir(int x, int y) const;

	//return nullptr if invalid coords 
	//(may cause chunk load)
	BlockStruct* getBlockM(int x, int y) override;


	//return is block at coords is air and true if outside the map
	//(may cause chunk load)
	bool isAir(int x, int y)
	{
		auto b = getBlockM(x, y);
		return b == nullptr || b->isAir();
	}

	//automatically calls chunk.markdirty() to update graphics and call onNeighborBlockChange()
	void setBlockWithNotify(int x, int y, BlockStruct& block) override;

	// just changes block value of blockstruct (no notification)
	void setBlock(int x, int y, BlockStruct& block) override;
	void setBlock(int x, int y, int blockid) { setBlock(x, y, BlockStruct(blockid)); }
	void setBlockWithNotify(int x, int y, int blockid) { setBlockWithNotify(x, y, BlockStruct(blockid)); }


	//automatically calls chunk.markdirty() to update graphics and call onNeighbourWallChange()
	void setWall(int x, int y, int wall_id) override;

	// ====buffering block changes====
	//opens block set session
	//you can call many setblock()
	//then you need to flush changes flushBlockSet()
	void beginBlockSet()
	{
		ASSERT(!m_edit_buffer_enable, "Called beginedit without calling flushEdit() beforehand");
		m_edit_buffer_enable = true;
	}

	//update all setblock block states
	void flushBlockSet();

	//==========INFO==================================================

	std::unordered_map<int, int>& getMap() { return m_local_offset_header_map; }
	auto& getHeaders() const { return m_chunk_headers; }
	long long getWorldTicks() const { return m_info.time; }
	WorldTime getWorldTime() const { return WorldTime(m_info.time); }
	std::string getName() const { return m_info.name; }
	const WorldInfo& getInfo() const { return m_info; };
	const std::string& getFilePath() const { return m_file_path; }
	auto& modifyInfo() { return m_info; }
	NBT& getWorldNBT() { return m_world_nbt; }
	WorldGen& getWorldGen() { return m_gen; }

	//================ENTITY========================================

	auto& getNBTSaver() { return m_nbt_saver; }
	EntityManager& getEntityManager() { return m_entity_manager; }
	ParticleManager** particleManager() { return &m_particle_manager; }
	
	void spawnParticle(ParticleID id, const glm::vec2& pos, const glm::vec2& speed, const glm::vec2& acc, float rotation = 0, half_int texturePos = half_int(-1, -1));

	// spawn break particles around block
	// amount in interval 1 - 32 is probability of spawning one particle
	//		- 1 means smallest probability of spawning
	//		- 32 means every particle is spawned
	void spawnBlockBreakParticles(int x, int y,int amount=32);


	WorldEntity* getLoadedEntity(EntityID id)
	{
		return m_entity_manager.entity(id);
	}

	WorldEntity* getLoadedTileEntity(int x, int y)
	{
		auto f = m_tile_entity_map.find(Phys::toInt64(x,y));
		if (f == m_tile_entity_map.end())
			return nullptr;
		return m_entity_manager.entity(f->second);
	}
	nd::temp_vector<WorldEntity*> getEntitiesInRadius(const glm::vec2& pos,float radius);
	nd::temp_vector<WorldEntity*> getEntitiesAtLocation(const glm::vec2& pos);
	const auto& getLoadedEntities() { return m_entity_array; }
	const auto& getLoadedTileEntities() { return m_tile_entity_map; }

	void loadEntity(WorldEntity* pEntity);

	void unloadEntity(EntityID id, bool isKilled = false);
	void unloadTileEntity(EntityID worldEntity, bool isKilled = false);

	// note: won't unload (tile)entity (from list), you need to remove it from local list yourself
	// calls only entity onUnload() and removes entity from entity_manager
	void unloadEntityNoDestruction(WorldEntity* entity, bool isKilled = false);

	EntityID spawnEntity(WorldEntity* pEntity);

	// kills normal or tile entity
	// never call on yourself
	// immediately calls ~() !
	// to kill yourself safely use entity.markDead() instead (~() will be called after update() of that entity)
	// if you know that entity is tilentity, use killTileEntity() instead
	void killEntity(EntityID id);

	// kills only tile entity
	// never call on yourself
	// immediately calls ~() !
	// to kill yourself safely use entity.markDead() instead (~() will be called after update() of that entity)
	void killTileEntity(EntityID id);

	std::vector<EntityID>::const_iterator beginEntities()
	{
		return m_entity_array.begin();
	}

	std::vector<EntityID>::const_iterator endEntities()
	{
		return m_entity_array.end();
	}

	std::vector<EntityID>::const_reverse_iterator rbeginEntities()
	{
		return m_entity_array.rbegin();
	}

	std::vector<EntityID>::const_reverse_iterator rendEntities()
	{
		return m_entity_array.rend();
	}

	//==============SERIALIZATION============================
public:
	// saves everything except for loaded chunks (and entities in those chunks)
	// return true if success
	JobAssignmentP saveWorld();

	// loads everything except for chunks
	// return true if success
	JobAssignmentP loadWorld();

	// creates all neccessary world files and stuff
	// no chunk generation
	// return true if success
	void genWorld();

	bool isChunkGenerated(int cx, int cy) const
	{
		return m_is_chunk_gen_map[getChunkSaveOffset(cx, cy)];
	}

	void markChunkGenerated(int cx, int cy)
	{
		m_is_chunk_gen_map.set(getChunkSaveOffset(cx, cy), true);
	}
};

