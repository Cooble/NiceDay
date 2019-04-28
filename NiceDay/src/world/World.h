#pragma once
#include "ndpch.h"
#include "Block.h"
#include "world/ITickable.h"
#include "WorldIO.h"
#define CHUNK_NOT_EXIST -1
#define CHUNK_BUFFER_LENGTH 20 //5*4

const int WORLD_CHUNK_SIZE = 64;
const int WORLD_CHUNK_BIT_SIZE = 6;

static const int BITS_FOR_CHUNK_LOC = 16;


class Chunk {
private:

	int m_x, m_y;
	//posxy

	BlockStruct m_blocks[WORLD_CHUNK_SIZE*WORLD_CHUNK_SIZE];
	bool m_loaded;
	friend class World;
	friend class WorldIO::Session;

public:
	Chunk();
	long last_save_time;
	inline int chunkID() const { return m_y << BITS_FOR_CHUNK_LOC | m_x; }
	inline bool isLoaded() const { return m_loaded; }


	inline BlockStruct& getBlock(int x, int y) { return m_blocks[y<<WORLD_CHUNK_BIT_SIZE | x]; }
	inline const BlockStruct& getBlock(int x, int y) const { return m_blocks[y << WORLD_CHUNK_BIT_SIZE | x]; }

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


class World : public ITickable
{
private:
	std::vector<Chunk> m_chunks;

	WorldInfo m_info;
	std::string m_file_path;
	std::unordered_map<int, int> m_local_offset_map;//chunkID,offsetInBuffer


private:


	void init();
public:
	World(const std::string& filePath, const char* name, int chunk_width, int chunk_height);
	World(const std::string& filePath, const WorldInfo* info);
	~World();

	void onUpdate();
	void tick();
	void onRender();
	inline bool isChunkLoaded(int x, int y) const { return m_local_offset_map.find(Chunk::getChunkIDFromChunkPos(x, y)) != m_local_offset_map.end(); }
	void unloadChunk(Chunk&);
	void unloadChunks(std::set<int>& chunkIds);
	void loadChunks(std::set<int>& chunkIds);
	void saveChunks(std::set<int>& chunkIds);
	void saveChunk(Chunk&);

	inline int getChunkSaveOffset(int x, int y) const { return y * m_info.chunk_width + x; };
	inline int getChunkSaveOffset(int id) const {
		int x,y;
		Chunk::getChunkPosFromID(id,x,y);
		return getChunkSaveOffset(x, y);
	};


	void unloadAllChunks();
	void saveAllChunks();
	Chunk &getChunk(int x, int y);
	int getChunkIndex(int x, int y);
	int getChunkIndex(int id);
	Chunk &loadChunk(int x, int y);
	void genWorld(long seed);//deprecated
	BlockStruct& getBlock(int x, int y);

	inline const std::unordered_map<int, int>::iterator& getChunkBegin() {
		return m_local_offset_map.begin();
	}
	inline std::unordered_map<int, int>& getMap() { return m_local_offset_map; }

	inline const std::unordered_map<int, int>::iterator& getChunkEnd() {
		return m_local_offset_map.end();
	}

	inline long getTime() const { return m_info.time; }
	inline const std::string getName() const { return m_info.name; }
	//returns copy of info!
	inline WorldInfo getInfo() const { return m_info; };
	inline const std::string& getFilePath() const { return m_file_path; }
	inline int getNumberOfLoadedChunks() const {
		return m_local_offset_map.size();
	}

};

