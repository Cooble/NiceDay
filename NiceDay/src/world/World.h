#pragma once
#include "ndpch.h"
#include "Block.h"
#define CHUNK_NOT_EXIST -1
#define CHUNK_BUFFER_LENGTH 20 //5*4

const int WORLD_CHUNK_SIZE = 64;


class Chunk {
private:
	union {
		struct {
			int m_x, m_y;
		};
		long long m_id;
	};//posxy

	BlockStruct m_blocks[WORLD_CHUNK_SIZE*WORLD_CHUNK_SIZE];
	bool m_loaded;
	friend class World;
	friend class WorldGen;

public:
	Chunk();
	long timestamp;
	inline bool isLoaded() const { return m_loaded; }
	inline BlockStruct& getBlock(int x, int y) { return m_blocks[y*WORLD_CHUNK_SIZE + x]; }
	inline const BlockStruct& getBlock(int x, int y) const { return m_blocks[y*WORLD_CHUNK_SIZE + x]; }

};
struct WorldInfo {
	long seed;
	std::string name;
	int chunk_width, chunk_height;

	long time;
};


class World
{
private:
	std::vector<Chunk> m_chunks;
	union {
		struct {
			long m_seed;
			std::string m_name;
			int m_chunk_width, m_chunk_height;

			long m_time;
		};
		WorldInfo m_worldInfo;
	};

private:

	void genChunk(int x, int y);
	Chunk& loadChunk(int x, int y);
	void unloadChunk(int x, int y);
	inline int getChunkOffset(int x, int y) { return y * m_chunk_width + x; };

public:
	World(std::string name, int chunk_width, int chunk_height);
	~World();

	void onUpdate();
	void onRender();

	Chunk& getChunk(int x, int y);
	int getChunkIndex(int x, int y);
	void genWorld(long seed);//deprecated
	BlockStruct& getBlock(int x, int y);

	long getTime();
	inline std::string getName() const { return m_name; };
	inline WorldInfo getInfo() const { return m_worldInfo; };

};

