#pragma once
#include "ndpch.h"
#include "WorldIO.h"
#include "block/Block.h"

#define CHUNK_NOT_EXIST -1
#define CHUNK_BUFFER_LENGTH 20 //5*4

const int WORLD_CHUNK_BIT_SIZE = 5;
const int WORLD_CHUNK_SIZE = 32;
const int WORLD_CHUNK_AREA = WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE;

static const int BITS_FOR_CHUNK_LOC = 16;

class Chunk {
private:

	int m_x, m_y;
	//posxy

	BlockStruct m_blocks[WORLD_CHUNK_SIZE*WORLD_CHUNK_SIZE];
	bool m_loaded;
	bool m_dirty;//if this flag is set in next update chunk graphics will be reloaded into chunkrenderer
	friend class World;
	friend class WorldIO::Session;

public:
	Chunk();
	long last_save_time;
	inline int chunkID() const { return m_y << BITS_FOR_CHUNK_LOC | m_x; }
	inline bool isLoaded() const { return m_loaded; }


	inline BlockStruct& getBlock(int x, int y) { return m_blocks[y<<WORLD_CHUNK_BIT_SIZE | x]; }
	inline void setBlock(int x, int y,BlockStruct& blok) { m_blocks[y<<WORLD_CHUNK_BIT_SIZE | x]=blok; }
	inline const BlockStruct& getBlock(int x, int y) const { return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x]; }
	inline bool isDirty()const { return m_dirty; }
	inline void markDirty(bool dirty) { m_dirty = dirty; }



	static inline int getChunkIDFromWorldPos(int x, int y) { return (y >> WORLD_CHUNK_BIT_SIZE) << BITS_FOR_CHUNK_LOC | (x >> WORLD_CHUNK_BIT_SIZE); }
	static inline int getChunkIDFromChunkPos(int x, int y) { return y << BITS_FOR_CHUNK_LOC | x; }
	static inline void getChunkPosFromID(int id, int& x, int& y) {
		x = id & ((1 << BITS_FOR_CHUNK_LOC) - 1);
		y = id >> BITS_FOR_CHUNK_LOC;
	}

};

struct WorldInfo {
	long seed;
	int chunk_width, chunk_height;
	long time;
	char name[100];
};

class World
{
private:
	std::vector<Chunk> m_chunks;

	WorldInfo m_info;
	std::string m_file_path;
	std::unordered_map<int, int> m_local_offset_map;//chunkID,offsetInBuffer
	BlockStruct m_air_block;

private:
	void init();
	void onBlocksChange(int x, int y,int deep);

public:
	inline static void getChunkCoords(float x, float y, unsigned int& cx, unsigned int& cy)
	{
		cx = getChunkCoord(x);
		cy = getChunkCoord(y);
	}
	inline static int getChunkCoord(float x)
	{
		return getChunkCoord((int)x);
	}
	inline static int getChunkCoord(int x)
	{
		return x >> WORLD_CHUNK_BIT_SIZE;
	}
public:
	World(const std::string& file_path, const char* name, int chunk_width, int chunk_height);
	World(const std::string& file_path, const WorldInfo* info);
	~World();

	void onUpdate();
	void tick();
	inline bool isChunkLoaded(int x, int y) const { return m_local_offset_map.find(Chunk::getChunkIDFromChunkPos(x, y)) != m_local_offset_map.end(); }
	void unloadChunk(Chunk&);
	void unloadChunks(std::set<int>& chunk_ids);
	void loadChunks(std::set<int>& chunk_ids);
	void saveChunks(std::set<int>& chunk_ids);
	void saveChunk(Chunk&);

	bool isValidBlock(int x, int y) const;

	inline int getChunkSaveOffset(int x, int y) const { return y * m_info.chunk_width + x; };
	inline int getChunkSaveOffset(int id) const {
		int x,y;
		Chunk::getChunkPosFromID(id,x,y);
		return getChunkSaveOffset(x, y);
	};

	void saveAllChunks();
	Chunk &getChunk(int x, int y);
	int getChunkIndex(int x, int y) const;
	int getChunkIndex(int id) const;
	Chunk &loadChunk(int x, int y);
	void genWorld(long seed);//deprecated

	//for reading the blocks 
	inline const BlockStruct& getBlock(int x, int y);
	const BlockStruct* getBlockPointer(int x, int y);
	//note any changes wont be visible in graphics ->use setBlock() instead
	BlockStruct& editBlock(int x, int y);
	//return is block at coords is air and true if outside the map
	bool isAir(int x, int y);

	//automatically calls chunk.markdirty() to update graphics and call onNeighbourBlockChange()
	void setBlock(int x, int y,BlockStruct&);
	void setBlock(int x, int y,int block_id);

	inline const std::unordered_map<int, int>::iterator& getChunkBegin() {return m_local_offset_map.begin();}
	inline std::unordered_map<int, int>& getMap() { return m_local_offset_map; }

	inline const std::unordered_map<int, int>::iterator& getChunkEnd() {return m_local_offset_map.end();}

    inline long getTime() const { return m_info.time; }
	inline std::string getName() const { return m_info.name; }
	//returns copy of info!
	inline WorldInfo getInfo() const { return m_info; };
	inline const std::string& getFilePath() const { return m_file_path; }
	inline int getNumberOfLoadedChunks() const {return m_local_offset_map.size();}

};

