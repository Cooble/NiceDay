#include "ndpch.h"
#include "WorldGen.h"
#include "world/World.h"
#include "world/biome/Biome.h"
#include "world/block/block_datas.h"
#include "world/gen/PriorGen.h"


static float clamp(float v, float min, float max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

static float smoothstep(float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

static float smootherstep(float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

float WorldGen::getTerrainHeight(int seed, float x)
{
	constexpr float epsilons[] //block distance between two points
	{
		40,
		20,
		10

	};
	constexpr float epsilonMagnitudes[]
	{
		10,
		5,
		2
	};

	float totalHeight = 0;
	for (int i = 0; i < sizeof(epsilons) / sizeof(float); ++i)
	{
		int index0 = (int)(x / epsilons[i]);
		int index1 = index0 + 1;
		//std::srand(seed + (index0 * 489 + 1456 * i) * 1000);
		std::srand(seed + (index0 * 489 + 1456) * 1000);
		float v0 = (std::rand() % 1024) / 1023.f;
		std::srand(seed + (index1 * 489 + 1456) * 1000);
		//std::srand(seed + (index1 * 489 + 1456 * i) * 1000);
		float v1 = (std::rand() % 1024) / 1023.f;

		totalHeight += ((smootherstep((x - index0 * epsilons[i]) / epsilons[i]) * (v1 - v0) + v0) * 2 - 1) *
			epsilonMagnitudes[i];
	}


	return totalHeight;
}

inline static bool& getFromMap(bool* m, int x, int y)
{
	ASSERT(x >= 0 && x < 2*WORLD_CHUNK_SIZE&&y >= 0 && y < 2*WORLD_CHUNK_SIZE, "Invalid chunkgen coords!");
	return m[y * WORLD_CHUNK_SIZE * 2 + x];
}

static int getNeighbourCount(bool* map, int x, int y)
{
	int out = 0;
	for (int xx = x - 1; xx < x + 2; ++xx)
	{
		for (int yy = y - 1; yy < y + 2; ++yy)
		{
			if (xx <= 0 || xx >= 2 * WORLD_CHUNK_SIZE || yy <= 0 || yy >= 2 * WORLD_CHUNK_SIZE)
				continue;
			out += getFromMap(map, xx, yy) ? 0 : 1;
		}
	}
	return out;
}

void WorldGen::genLayer0(World& w, Chunk& c)
{
	TimerStaper t("gen chunk");
	if (m_prior_gen == nullptr)
	{
		m_prior_gen = new PriorGen("file");
		m_prior_gen->gen(w.getInfo().seed, w.getInfo().terrain_level,
		                 w.getInfo().chunk_width * WORLD_CHUNK_SIZE, w.getInfo().chunk_height * WORLD_CHUNK_SIZE);
	}
	c.m_biome = BIOME_FOREST;
	for (int zz = 0; zz < WORLD_CHUNK_SIZE; ++zz)
	{
		for (int ww = 0; ww < WORLD_CHUNK_SIZE; ++ww)
		{
			auto worldx = c.m_x * WORLD_CHUNK_SIZE + ww;
			auto worldy = c.m_y * WORLD_CHUNK_SIZE + zz;
			c.block(ww, zz) = m_prior_gen->getBlock(worldx, worldy);
		}
	}
	int offsetX = c.m_x * WORLD_CHUNK_SIZE;
	int offsetY = c.m_y * WORLD_CHUNK_SIZE;
	half_int index = c.chunkID();

	/*TimerStaper t("gen chunk");
	static bool* map0 = new bool[WORLD_CHUNK_AREA * 4];
	static bool* map1 = new bool[WORLD_CHUNK_AREA * 4];
	srand(c.chunkID() * 123546+5);

	auto& info = w.getInfo();
	c.m_biome = BIOME_UNDERGROUND;
	if ((c.m_y + 1) * WORLD_CHUNK_SIZE > (info.terrain_level - 1))
		c.m_biome = BIOME_FOREST;
	if (c.m_x < 2)
		c.m_biome = BIOME_DIRT;
	bool torchPlaced = false;
	int torchX = rand() % WORLD_CHUNK_SIZE;
	if (rand() % 2 == 0)
		torchPlaced = true;//skip torch placement in 50% of cases
	int heightmap[WORLD_CHUNK_SIZE];
	for (int x = 0; x < WORLD_CHUNK_SIZE; ++x)
	{
		heightmap[x] = round(getTerrainHeight(seed, c.m_x * WORLD_CHUNK_SIZE+x));
	}
	srand(c.chunkID() * 654789 + 4);
	for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
	{
		for (int y = WORLD_CHUNK_SIZE-1; y >= 0; --y)
		{
			auto worldx = c.m_x * WORLD_CHUNK_SIZE + x;
			auto worldy = c.m_y * WORLD_CHUNK_SIZE + y;
			auto& block = c.block(x, y);

		
			int terrain = info.terrain_level + heightmap[x];
			//if (x == 0 || y == 0)
			//	block.block_id = 0;
			if (block.block_id == BLOCK_TORCH)
				continue;
			if (worldy > terrain)
				block.block_id = BLOCK_AIR;
			else if (worldy == terrain) {
				if (torchX == x && !torchPlaced) {
					if (y != WORLD_CHUNK_SIZE - 1)
					{
						torchPlaced = true;
						c.block(x, y + 1).block_id = BLOCK_TORCH;
					}
				}
				if (y != (WORLD_CHUNK_SIZE - 1)) 
				{
					switch (std::rand() % 8)
					{
					case 0:
						c.block(x, y + 1).block_id = BLOCK_FLOWER;
						c.block(x, y + 1).block_metadata = std::rand() % 8;
						break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						c.block(x, y + 1).block_id = BLOCK_GRASS_PLANT;
						c.block(x, y + 1).block_metadata = std::rand() % 4;
						break;
					}
				}
				block.block_id = BLOCK_GRASS;
			}
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

	

	//prepare map
	for (int x = 0; x < WORLD_CHUNK_SIZE*2; ++x)
	{
		for (int y = 0; y < WORLD_CHUNK_SIZE*2; ++y)
		{
			int xxxx = (x + offsetX - WORLD_CHUNK_SIZE / 2);
			int yyyy = (y + offsetY - WORLD_CHUNK_SIZE / 2);
			std::srand(123456*xxxx + 5645646*yyyy+ xxxx* yyyy);//sranda innit?
			getFromMap(map0,x,y) = (std::rand() % 10)>5;
		}
	}
	bool* curMap = map0;
	bool* nextMap = map1;
		//time for cellular automata
	for (int i = 0; i < 3; ++i)
	{
		for (int x = 0; x < WORLD_CHUNK_SIZE * 2; ++x)
		{
			for (int y = 0; y < WORLD_CHUNK_SIZE * 2; ++y)
			{
				int ieger = getNeighbourCount(curMap, x, y);
				getFromMap(nextMap, x, y) = ieger < 7;
			}
		}
		auto matic = curMap;
		curMap = nextMap;
		nextMap = matic;
	}
	for (int x = WORLD_CHUNK_SIZE/2; x < WORLD_CHUNK_SIZE*1.5; ++x)
	{
		for (int y = WORLD_CHUNK_SIZE/2; y < WORLD_CHUNK_SIZE*1.5; ++y)
		{
			if (!getFromMap(curMap,x,y))
				c.block(x - WORLD_CHUNK_SIZE / 2, y - WORLD_CHUNK_SIZE / 2).block_id = BLOCK_AIR;
		}
	}

	int xxxx = (offsetX);
	int yyyy = (offsetY);
	std::srand(564 * xxxx + 78954 * yyyy + xxxx * yyyy);//sranda innit?
	int randX = std::rand() % 16;
	int randY = std::rand() % 16;
	if(c.block(randX,randY).block_id==BLOCK_STONE)
	{
		if(randY!=0&& c.block(randX, randY).block_id!=BLOCK_AIR)
		{
			c.block(randX, randY).block_id = BLOCK_TORCH;
		}
	}
	*/
	//update block states
	for (int y = 1; y < WORLD_CHUNK_SIZE - 1; y++)
	{
		for (int x = 1; x < WORLD_CHUNK_SIZE - 1; x++)
			//boundaries will be updated after all other adjacent chunks are generated
		{
			auto& block = c.block(x, y);

			auto worldx = offsetX + x;
			auto worldy = offsetY + y;
			if (!block.isAir())
				BlockRegistry::get().getBlock(block.block_id).onNeighbourBlockChange(*w.getTotalBlockAccess(), worldx, worldy);
			if (!block.isWallFree())
				BlockRegistry::get().getWall(block.wallID()).onNeighbourWallChange(*w.getTotalBlockAccess(), worldx, worldy);
		}
	}
	c.last_save_time = w.getWorldTicks(); //mark it as was generated
}
