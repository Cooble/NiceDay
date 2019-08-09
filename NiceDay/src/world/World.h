#pragma once
#include "ndpch.h"
#include "WorldIO.h"
#include "LightCalculator.h"
#include "WorldGen.h"
#include "block/Block.h"
#include "entity/WorldEntity.h"

#define CHUNK_NOT_EXIST -1
#define CHUNK_BUFFER_LENGTH 100 //5*4


constexpr int DYNAMIC_ID_ENITY_MANAGER = std::numeric_limits<int>::max() - 0;
constexpr int DYNAMIC_ID_WORLD_NBT = std::numeric_limits<int>::max() - 1;

const int WORLD_CHUNK_BIT_SIZE = 5;
const int WORLD_CHUNK_SIZE = 32;
const int WORLD_CHUNK_AREA = WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE;

static const int BITS_FOR_CHUNK_LOC = 16;

constexpr int CHUNK_LOADED_FLAG = BIT(0);
constexpr int CHUNK_DIRTY_FLAG = BIT(1); //in next update chunk graphics will be reloaded into chunkrenderer
constexpr int CHUNK_LOCKED_FLAG = BIT(2); //dont unload this chunk its being worked on by main thread
constexpr int CHUNK_LIGHT_LOCKED_FLAG = BIT(3); //dont unload this chunk its being worked on by light thread

typedef int ChunkID;
class Chunk
{
private:
	int m_x, m_y;
	//posxy

	BlockStruct m_blocks[WORLD_CHUNK_AREA];
	uint8_t m_light_levels[WORLD_CHUNK_AREA];
	uint16_t m_flags = 0;
	int m_biome;

	//multithreading light
	uint32_t m_main_thread_fence = 0;
	uint32_t m_light_thread_fence = 0;

public:
	friend class World;
	friend class WorldIO::Session;
	friend class WorldGen;

	Chunk();
	long long last_save_time;
	inline int getCX() const { return m_x; }
	inline int getCY() const { return m_y; }
	inline ChunkID chunkID() const { return half_int(m_x, m_y); }
	inline bool isLoaded() const { return m_flags & CHUNK_LOADED_FLAG; } //this chunk shell contains loaded chunk

	// cannot unload locked chunk
	inline bool isLocked() const
	{
		return (m_flags & CHUNK_LOCKED_FLAG) | (m_main_thread_fence != m_light_thread_fence);
	}

	inline bool isLightLocked() const { return m_flags & CHUNK_LIGHT_LOCKED_FLAG; }
	inline bool isDirty() const { return m_flags & CHUNK_DIRTY_FLAG; }
	inline bool isGenerated() const { return last_save_time != 0; } //worldgen has generated it
	inline void lock(bool lock)
	{
		if (lock) m_flags |= CHUNK_LOCKED_FLAG;
		else m_flags &= ~CHUNK_LOCKED_FLAG;
	}

	inline void lightLock() { ++m_main_thread_fence; }
	inline void lightUnlock() { ++m_light_thread_fence; }

	// used as fence with main-light thread resource sharing
	inline void lightLock(bool lock)
	{
		if (lock) m_flags |= CHUNK_LIGHT_LOCKED_FLAG;
		else m_flags &= ~CHUNK_LIGHT_LOCKED_FLAG;
	}

	inline void setLoaded(bool loaded)
	{
		if (loaded) m_flags |= CHUNK_LOADED_FLAG;
		else m_flags &= ~CHUNK_LOADED_FLAG;
	}

	inline BlockStruct& block(int x, int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	inline const BlockStruct& block(int x, int y) const { return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x]; }

	inline uint8_t& lightLevel(int x, int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_light_levels[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	inline uint8_t lightLevel(int x, int y) const
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_light_levels[y << WORLD_CHUNK_BIT_SIZE | x];
	}

	inline void setBlock(int x, int y, BlockStruct& blok) { m_blocks[y << WORLD_CHUNK_BIT_SIZE | x] = blok; }

	inline void markDirty(bool dirty)
	{
		if (dirty) m_flags |= CHUNK_DIRTY_FLAG;
		else m_flags &= ~CHUNK_DIRTY_FLAG;
	}

	inline int getBiome() const { return m_biome; }

	inline void setBiome(int biome_id) { m_biome = biome_id; }

	inline Phys::Rectangle getChunkRectangle() const
	{
		return Phys::Rectangle::createFromDimensions(m_x*WORLD_CHUNK_SIZE, m_y*WORLD_CHUNK_SIZE, WORLD_CHUNK_SIZE, WORLD_CHUNK_SIZE);
	}

	static inline int getChunkIDFromWorldPos(int wx, int wy)
	{
		return (wy >> WORLD_CHUNK_BIT_SIZE) << BITS_FOR_CHUNK_LOC | (wx >> WORLD_CHUNK_BIT_SIZE);
	}

	static inline int getChunkIDFromChunkPos(int cx, int cy) { return cy << BITS_FOR_CHUNK_LOC | cx; }

	static inline void getChunkPosFromID(int id, int& cx, int& cy)
	{
		cx = id & ((1 << BITS_FOR_CHUNK_LOC) - 1);
		cy = id >> BITS_FOR_CHUNK_LOC;
	}
};

enum class Day : int
{
	MON=0,TUE,WED,THR,FRI,SAT,SUN
};
constexpr long TICKS_PER_MINUTE = 1;//every two seconds we have one game minute
struct WorldTime
{
	long long m_ticks;
	WorldTime(long long ticks):m_ticks(ticks){}

	inline float hours() const
	{
		return (float)(m_ticks % (TICKS_PER_MINUTE * 60 * 24)) / TICKS_PER_MINUTE / 60;
	}
	inline float minutes() const
	{
		return (float)(m_ticks % (TICKS_PER_MINUTE * 60)) / TICKS_PER_MINUTE;
	}
	inline Day dayOfWeek() const
	{
		return Day((m_ticks % (TICKS_PER_MINUTE * 60 * 24 * 7))/(TICKS_PER_MINUTE * 60 * 24));
	}
	
	inline bool isNight() const
	{
		auto hour = hours();
		return hour > 19.5 || hour < 5.5;
	}
};
struct WorldInfo
{
	long seed;
	int chunk_width, chunk_height;
	int terrain_level;
	long long time;
	char name[100];
	EntityID player_id;
	ChunkID playerChunk;
};

class World
{
public:
	inline static int toChunkCoord(float x) { return toChunkCoord((int)x); }
	inline static int toChunkCoord(int x) { return x >> WORLD_CHUNK_BIT_SIZE; }
private:
	friend class WorldIO::Session;
	friend class WorldGen;
	LightCalculator m_light_calc;
	WorldGen m_gen;
	std::vector<Chunk> m_chunks;
	NBT m_world_nbt;

	WorldInfo m_info;
	std::string m_file_path;
	std::unordered_map<int, int> m_local_offset_map; //chunkID,offsetInBuffer
	BlockStruct m_air_block=0;

	EntityManager m_entity_manager;
	WorldIO::DynamicSaver m_nbt_saver;

	std::vector<EntityID> m_entity_array;
	std::vector<EntityID> m_entity_array_buff;


	bool m_edit_buffer_enable = false;
	std::queue<std::pair<int, int>> m_edit_buffer; //location x,y for editted blocks

private:
	void init();
	void onBlocksChange(int x, int y, int deep = 0);
	void onWallsChange(int xx, int yy, BlockStruct& blok);
	int getNextFreeChunkIndex(int startSearchIndex = 0);
	void genChunks(std::set<int>& toGenChunks);
	void loadLightResources(int x, int y);

public:
	World(std::string file_path, const WorldInfo& info);
	~World();

	inline LightCalculator& getLightCalculator() { return m_light_calc; }
	void onUpdate();
	void tick();


	inline bool isBlockValid(int x, int y) const
	{
		return x >= 0 && y >= 0 && x < getInfo().chunk_width * WORLD_CHUNK_SIZE && y < getInfo().chunk_height *
			WORLD_CHUNK_SIZE;
	}

	inline bool isChunkValid(int x, int y) const
	{
		return x >= 0 && y >= 0 && x < getInfo().chunk_width && y < getInfo().chunk_height;
	}
	glm::vec4 getSkyColor();
	bool isChunkGenerated(int x, int y);

	inline int getChunkSaveOffset(int x, int y) const { return y * m_info.chunk_width + x; }

	inline int getChunkSaveOffset(int id) const
	{
		return getChunkSaveOffset(((half_int*)&id)->x, ((half_int*)&id)->y);
	};
	//==============CHUNK METHODS========================================================================

	Chunk& getChunk(int x, int y);

	//return nullptr if chunk is not loaded or invalid coords 
	//(won't cause chunk load)
	const Chunk* getLoadedChunkPointer(int x, int y) const;
	Chunk* getLoadedChunkPointerNoConst(int cx, int cy);
	int getChunkIndex(int x, int y) const;
	int getChunkIndex(int id) const;
	Chunk& loadChunk(int x, int y);

	inline bool isChunkLoaded(int x, int y) const { return getChunkIndex(x, y) != -1; }
	void unloadChunk(Chunk&);
	void unloadChunks(std::set<int>& chunk_ids);
	void updateChunkBounds(int x, int y, int bitBounds);
	void loadChunksAndGen(std::set<int>& toLoadChunks);

	//==============BLOCK METHODS=======================================================================

	//return nullptr if block is not loaded or invalid coords 
	//(won't cause chunk load)
	const BlockStruct* getLoadedBlockPointer(int x, int y) const;

	//return nullptr if invalid coords 
	//(may cause chunk load)
	const BlockStruct* getBlockPointer(int x, int y);

	//for reading the blocks 
	//(may cause chunk load)
	inline const BlockStruct& getBlock(int x, int y) { return editBlock(x, y); }

	//return is block at coords is air and true if outside the map
	//(may cause chunk load)
	bool isAir(int x, int y);


	//any changes wont be visible in graphics ->use setBlock() instead 
	//(may cause chunk load)
	BlockStruct& editBlock(int x, int y);

	//automatically calls chunk.markdirty() to update graphics and call onNeighbourBlockChange()
	//(may cause chunk load)
	void setBlock(int x, int y, BlockStruct&);


	//automatically calls chunk.markdirty() to update graphics and call onNeighbourBlockChange()
	//(may cause chunk load)
	void setBlock(int x, int y, int block_id);

	//opens block set session
	//you can call many setblock()
	//then you need to flush changes flushBlockSet()
	inline void beginBlockSet()
	{
		ASSERT(!m_edit_buffer_enable, "Called beginedit without calling flushEdit() beforehand");
		m_edit_buffer_enable = true;
	}

	//update all setblock block states
	void flushBlockSet();

	//automatically calls chunk.markdirty() to update graphics and call onNeighbourWallChange()
	//(may cause chunk load)
	void setWall(int x, int y, int wall_id);

	//==========INFO==================================================

	inline std::unordered_map<int, int>& getMap() { return m_local_offset_map; }

	inline long long getWorldTicks() const { return m_info.time; }
	inline WorldTime getWorldTime() const { return WorldTime(m_info.time); }
	inline std::string getName() const { return m_info.name; }
	inline const WorldInfo& getInfo() const { return m_info; };
	inline const std::string& getFilePath() const { return m_file_path; }
	inline auto& modifyInfo() { return m_info; }
	inline NBT& getWorldNBT() { return m_world_nbt; }

	//================ENTITY========================================

	inline auto& getNBTSaver() { return m_nbt_saver; }
	inline EntityManager& getEntityManager() { return m_entity_manager; }

	inline WorldEntity* getLoadedEntity(EntityID id)
	{
		return m_entity_manager.entity(id);
	}

	inline const auto& getLoadedEntities() { return m_entity_array; }

	void loadEntity(WorldEntity* pEntity);

	void unloadEntity(EntityID worldEntity);

	EntityID spawnEntity(WorldEntity* pEntity);

	void killEntity(EntityID id);

	inline std::vector<EntityID>::const_iterator beginEntities()
	{
		return m_entity_array.begin();
	}

	inline std::vector<EntityID>::const_iterator endEntities()
	{
		return m_entity_array.end();
	}


	inline std::vector<EntityID>::const_reverse_iterator rbeginEntities()
	{
		return m_entity_array.rbegin();
	}

	inline std::vector<EntityID>::const_reverse_iterator rendEntities()
	{
		return m_entity_array.rend();
	}

	//==============SERIALIZATION============================
public:
	// saves everything except for loaded chunks (and entities in those chunks)
	// return true if success
	bool saveWorld();

	// loads everything except for chunks
	// return true if success
	bool loadWorld();

	// creates all neccessary world files and stuff
	// no chunk generation
	// return true if success
	bool genWorld();
};


