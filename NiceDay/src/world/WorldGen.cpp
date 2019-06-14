﻿#include "ndpch.h"
#include "WorldGen.h"
#include "World.h"
#include "biome/Biome.h"
#include "block/block_datas.h"

static float clamp(float v,float min, float max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;

}

static float smoothstep(float x) {
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

static float smootherstep(float x) {
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

float WorldGen::getTerrainHeight(int seed, float x)
{
	constexpr float epsilons[]//block distance between two points
	{
		60,
		10,
		5
		
	};
	constexpr float epsilonMagnitudes[]
	{
		40,
		10,
		3
	};

	float totalHeight=0;
	for (int i = 0; i < sizeof(epsilons)/sizeof(float); ++i)
	{
		int index0 = (int)(x / epsilons[i]);
		int index1 = index0+1;
		std::srand(seed + index0*100000);
		float v0 = (float)(std::rand() % 1000) /1000.0f;
		std::srand(seed + index1 *100000);
		float v1 = (float)(std::rand() % 1000) / 1000.0f;
		totalHeight+=((smootherstep((x-index0* epsilons[i])/epsilons[i])*(v1-v0)+v0)*2-1) * epsilonMagnitudes[i];
	}



	return totalHeight;



}

void WorldGen::gen(int seed, World* w, Chunk& c)
{
	auto& info = w->getInfo();
	c.m_biome = BIOME_UNDERGROUND;
	if ((c.m_y + 1) * WORLD_CHUNK_SIZE > (info.terrain_level - 1))
		c.m_biome = BIOME_FOREST;
	if (c.m_x < 2)
		c.m_biome = BIOME_DIRT;
	for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
	{
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.getBlock(x, y);

			auto worldx = c.m_x * WORLD_CHUNK_SIZE + x;
			auto worldy = c.m_y * WORLD_CHUNK_SIZE + y;
			int terrain = info.terrain_level + getTerrainHeight(seed, worldx);
			//if (x == 0 || y == 0)
			//	block.block_id = 0;
			if (worldy > terrain)
				block.block_id = BLOCK_AIR;
			else if (worldy == terrain)
				block.block_id = BLOCK_GRASS;
			else if (worldy == terrain-1)
				block.block_id = BLOCK_DIRT;
			else if (worldy > info.terrain_level - 8)
			{
				//5 block of dirt
				block.block_id = BLOCK_DIRT;
				block.setWall(WALL_DIRT);
			}
			else if (c.m_x < 2)
				block.block_id = BLOCK_ADAMANTITE;
			else
			{
				block.block_id = BLOCK_STONE;
				block.setWall(WALL_STONE);
			}
		}
	}

	//this worldgen is lame
	std::vector<std::pair<int, int>> robs;
	std::vector<std::pair<int, int>> newRobs;
	auto r1 = &robs;
	auto r2 = &newRobs;
	std::srand((seed+1)*c.chunkID() * 100);
	int length = std::rand() % 15+3;
	r1->emplace_back(std::make_pair(std::rand() % 16, std::rand() % 16));
	for (int i = 0; i < length; ++i)
	{
		for(auto t: *r1)
		{
			int& robX = t.first;
			int& robY = t.second;

			int xMove = std::rand() % 3-1;
			robX += xMove;
			robY += xMove==0? std::rand() % 3 - 1:0;

			if (robX < 0 || robX >= WORLD_CHUNK_SIZE)
				continue;
			if (robY < 0 || robY >= WORLD_CHUNK_SIZE)
				continue;
			if (c.getBlock(robX, robY).block_id != 0)
			{
				if (std::rand() % 10 == 0)
					r2->emplace_back(std::make_pair(robX, robY));
				c.getBlock(robX, robY).block_id = std::rand()%2==0?BLOCK_ADAMANTITE:BLOCK_GOLD;
				r2->emplace_back(std::make_pair(robX, robY));
			}
		}
		auto t = r1;
		r1 = r2;
		r2 = t;
		
		
	}

	int offsetX = c.m_x * WORLD_CHUNK_SIZE;
	int offsetY = c.m_y * WORLD_CHUNK_SIZE;
	//update block states
	for (int x = 1; x < WORLD_CHUNK_SIZE-1; x++)//boundaries will be updated after all other adjacent chunks are generated
	{
		for (int y = 1; y < WORLD_CHUNK_SIZE-1; y++)
		{
			auto& block = c.getBlock(x, y);

			auto worldx = offsetX + x;
			auto worldy = offsetY + y;
			if (!block.isAir())
				BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(w, worldx, worldy);
			if(!block.isWallFree())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(w, worldx, worldy);

		}
	}
	c.last_save_time = w->getTime();//mark it as was generated
}
