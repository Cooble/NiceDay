#pragma once
#include "ndpch.h"
#include "WorldIO.h"
#include "LightCalculator.h"
#include "WorldGen.h"
#include "block/Block.h"
#include "entity/WorldEntity.h"

#define CHUNK_NOT_EXIST -1
#define CHUNK_BUFFER_LENGTH 50 //5*4



const int WORLD_CHUNK_BIT_SIZE = 5;
const int WORLD_CHUNK_SIZE = 32;
const int WORLD_CHUNK_AREA = WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE;

static const int BITS_FOR_CHUNK_LOC = 16;

constexpr int CHUNK_LOADED_FLAG = BIT(0);
constexpr int CHUNK_DIRTY_FLAG = BIT(1);//in next update chunk graphics will be reloaded into chunkrenderer
constexpr int CHUNK_LOCKED_FLAG = BIT(2);//dont unload this chunk its being worked on

class Chunk {
private:

	int m_x, m_y;
	//posxy

	BlockStruct m_blocks[WORLD_CHUNK_AREA];
	uint8_t m_light_levels[WORLD_CHUNK_AREA];
	uint16_t m_flags;
	friend class World;
	friend class WorldIO::Session;
	friend class WorldGen;
	int m_biome;

public:
	Chunk();
	long long last_save_time;
	inline int chunkID() const { return half_int(m_x,m_y); }
	inline bool isLoaded() const { return m_flags & CHUNK_LOADED_FLAG; }//this chunk shell contains loaded chunk
	inline bool isLocked() const { return  m_flags & CHUNK_LOCKED_FLAG; }//cannot unload locked chunk
	inline bool isDirty()const { return m_flags & CHUNK_DIRTY_FLAG; }
	inline bool isGenerated() const { return last_save_time != 0; }//worldgen has generated it
	inline void lock(bool lock)
	{
		if (lock) m_flags |= CHUNK_LOCKED_FLAG;
		else m_flags &= ~CHUNK_LOCKED_FLAG;
	}
	inline void setLoaded(bool loaded)
	{
		if (loaded) m_flags |= CHUNK_LOADED_FLAG;
		else m_flags &= ~CHUNK_LOADED_FLAG;
	}

	inline BlockStruct& getBlock(int x, int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x];
	}
	inline const BlockStruct& getBlock(int x, int y) const { return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x]; }
	inline uint8_t& getLightLevel(int x,int y)
	{
		ASSERT(x >= 0 && x < WORLD_CHUNK_SIZE&&y >= 0 && y < WORLD_CHUNK_SIZE, "Invalid chunk coords!");
		return m_light_levels[y << WORLD_CHUNK_BIT_SIZE | x];
	}
	inline uint8_t getLightLevel(int x,int y) const
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
	inline void setBiome(int biome_id) { m_biome= biome_id; }



	static inline int getChunkIDFromWorldPos(int wx, int wy) { return (wy >> WORLD_CHUNK_BIT_SIZE) << BITS_FOR_CHUNK_LOC | (wx >> WORLD_CHUNK_BIT_SIZE); }
	static inline int getChunkIDFromChunkPos(int cx, int cy) { return cy << BITS_FOR_CHUNK_LOC | cx; }
	static inline void getChunkPosFromID(int id, int& cx, int& cy) {
		cx = id & ((1 << BITS_FOR_CHUNK_LOC) - 1);
		cy = id >> BITS_FOR_CHUNK_LOC;
	}

};

struct WorldInfo {
	long seed;
	int chunk_width, chunk_height;
	int terrain_level;
	long time;
	char name[100];
};

class World
{
public:
	inline static int toChunkCoord(float x){	return toChunkCoord((int)x);}
	inline static int toChunkCoord(int x){	return x >> WORLD_CHUNK_BIT_SIZE;}
private:
	friend class WorldIO::Session;
	friend class WorldGen;
	LightCalculator  m_light_calc;
	WorldGen m_gen;
	std::vector<Chunk> m_chunks;

	WorldInfo m_info;
	std::string m_file_path;
	std::unordered_map<int, int> m_local_offset_map;//chunkID,offsetInBuffer
	BlockStruct m_air_block;

	EntityManager m_entity_manager;

	std::vector<EntityID> m_entity_array;

private:
	void init();
	void onBlocksChange(int x, int y, int deep);
	void onWallsChange(int xx, int yy, BlockStruct& blok);
	int getNextFreeChunkIndex(int startSearchIndex = 0);
	void genChunks(std::set<int>& toGenChunks);


public:
	World(std::string file_path, const char* name, int chunk_width, int chunk_height);
	World(std::string file_path, const WorldInfo* info);
	~World();

	inline LightCalculator& getLightCalculator() { return m_light_calc; }
	void onUpdate();
	void tick();


	inline bool isBlockValid(int x, int y) const{return x >= 0 && y >= 0 && x < getInfo().chunk_width * WORLD_CHUNK_SIZE && y < getInfo().chunk_height *WORLD_CHUNK_SIZE;}
	inline bool isChunkValid(int x, int y) const{ return x >= 0 && y >= 0 && x < getInfo().chunk_width && y < getInfo().chunk_height; }


	inline int getChunkSaveOffset(int x, int y) const { return y * m_info.chunk_width + x; };
	inline int getChunkSaveOffset(int id) const {
		return getChunkSaveOffset(((half_int*)&id)->x, ((half_int*)&id)->y);
	};
	//==============CHUNK METHODS========================================================================

	Chunk& getChunk(int x, int y);

	//return nullptr if chunk is not loaded or invalid coords 
	//(won't cause chunk load)
	const Chunk* getLoadedChunkPointer(int x,int y) const;
	Chunk* getLoadedChunkPointerNoConst(int cx, int cy);
	int getChunkIndex(int x, int y) const;
	int getChunkIndex(int id) const;
	Chunk &loadChunk(int x, int y);

	inline bool isChunkLoaded(int x, int y) const { return getChunkIndex(x, y) != -1; }
	void unloadChunk(Chunk&);
	void unloadChunks(std::set<int>& chunk_ids);
	void updateChunkBounds(int x, int y,int bitBounds);
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
	inline const BlockStruct& getBlock(int x, int y){return editBlock(x, y);}

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

	//automatically calls chunk.markdirty() to update graphics and call onNeighbourWallChange()
	//(may cause chunk load)
	void setWall(int x, int y, int wall_id);

	//==================================================================================================

	inline std::unordered_map<int, int>& getMap() { return m_local_offset_map; }

	inline long getTime() const { return m_info.time; }
	inline std::string getName() const { return m_info.name; }
	inline const WorldInfo& getInfo() const { return m_info; };
	inline const std::string& getFilePath() const { return m_file_path; }

	//ENTITY============================================================================================

	inline EntityManager& getEntityManager() { return m_entity_manager; }

	void spawnEntity(WorldEntity* worldEntity);

};



