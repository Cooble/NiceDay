#include "ndpch.h"
#include "WorldGen.h"
#include <iostream>
#include <fstream>

//in bytes
static const int HEADER_SIZE = 1000;
/**

WorldSaveHeader:
-WorldInfo
-Map of chunks? nope
-Chunks

*/
WorldGen::WorldGen()
{
}


WorldGen::~WorldGen()
{
}

World* WorldGen::genWorld(const WorldGenInfo & info)
{
	World* ww = new World(info.world_name, info.chunk_width, info.chunk_height);
	World& w = *ww;

	//creating world file
	auto myfile = std::fstream(info.world_name + ".world", std::ios::out | std::ios::binary);
	myfile.write((const char*)&w.getInfo(), sizeof(WorldInfo));
	auto buff = new char[HEADER_SIZE - sizeof(WorldInfo)];
	myfile.write(buff, HEADER_SIZE - sizeof(WorldInfo));//add some space to fill the HEADER_SIZE
	delete buff;

	for (int x = 0; x < info.chunk_width; x++)
	{
		for (int y = 0; y < info.chunk_height; y++)
		{
			Chunk c;
			c.m_x = x;
			c.m_y = y;
			c.timestamp = 0;

			myfile.write((const char*)&c, sizeof(Chunk));
		}

	}
	myfile.close();
	//generating world
	w.genWorld(info.seed);
	return ww;
}
World* WorldGen::loadWorld(const std::string& path)
{
	auto myfile = std::fstream(path + ".world", std::ios::in | std::ios::binary);
	WorldInfo info;
	myfile.read((char*)&info, sizeof(WorldInfo));

	myfile.close();

	World* ww = new World(info.name, info.chunk_width, info.chunk_height);
	return ww;
}
void WorldGen::loadChunk(const std::string& path, Chunk* chunk, int offset) {
	auto myfile = std::fstream(path + ".world", std::ios::in | std::ios::binary);
	myfile.seekg(HEADER_SIZE+sizeof(Chunk)*offset, std::ios::beg);
	myfile.read((char*)chunk, sizeof(Chunk));
	myfile.close();

}
void WorldGen::saveChunk(const std::string& path, Chunk* chunk, int offset) {
	auto myfile = std::fstream(path + ".world", std::ios::out | std::ios::binary);
	myfile.seekp(HEADER_SIZE + sizeof(Chunk)*offset, std::ios::beg);
	myfile.write((char*)chunk, sizeof(Chunk));
	myfile.close();
}
