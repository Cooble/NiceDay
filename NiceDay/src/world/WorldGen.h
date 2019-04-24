#pragma once
#include "ndpch.h"
#include "World.h"

#define WORLD_SAVE_FOLDER "saves/"
struct WorldGenInfo {
	std::string world_name;
	int chunk_width, chunk_height;
	long seed;

};

class WorldGen
{
private:
	WorldGen();
public:
	~WorldGen();
	static World* genWorld(const WorldGenInfo& info);
	static World* loadWorld(const std::string& path);

	static void loadChunk(const std::string& path, Chunk* chunk, int offset);
	static void saveChunk(const std::string& path, Chunk* chunk, int offset);


};

