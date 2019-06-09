#include "ndpch.h"
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
	auto t = TimerStaper("chunkgen sucks");

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
			if (x == 0 || y == 0)
				block.block_id = 0;
			else if (worldy > terrain)
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
			block.setCustomBit(true);
		}
	}
	//update block states
	for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
	{
		for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
		{
			auto& block = c.getBlock(x, y);

			auto worldx = c.m_x * WORLD_CHUNK_SIZE + x;
			auto worldy = c.m_y * WORLD_CHUNK_SIZE + y;
			if (!block.isAir())
				BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(w, worldx, worldy);
			if(!block.isWallFree())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(w, worldx, worldy);

		}
	}
}
