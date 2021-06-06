#include "ndpch.h"
#include "PriorGen.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "world/block/basic_blocks.h"
#include "world/block/block_datas.h"
#include "world/World.h"
//#include "OpenSimplexNoise.hpp"
#include "TreeGen.h"
using namespace nd;


PriorGen::PriorGen(const std::string& filePath)
	: m_file_path(filePath)/*, m_noise(nullptr), m_noise_2(nullptr)*/
{
}

static float clamp(float v, float min, float max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

PriorGen::~PriorGen()
{
	if (m_map)
	{
		free(m_map);
		free(m_pixels);
	}
}

static float smootherstep(float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

static float smoothstep(float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp(x, 0.0, 1.0);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}


float PriorGen::getTerrainHeight(int seed, float x)
{
	constexpr float epsilons[]//block distance between two points
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
		std::srand(seed + (index0 * 489 + 1456 * i) * 1000);
		float v0 = (std::rand() % 1024) / 1024.f;
		std::srand(seed + (index1 * 489 + 1456 * i) * 1000);
		float v1 = (std::rand() % 1024) / 1024.f;

		totalHeight += ((smootherstep((x - index0 * epsilons[i]) / epsilons[i]) * (v1 - v0) + v0) * 2 - 1) *
			epsilonMagnitudes[i];
	}


	return totalHeight;
}

void PriorGen::gen(uint64_t seed,int terrainLevel, int width, int height)
{
	m_seed = seed;
	m_width = width;
	m_height = height;
	m_terrain_level = terrainLevel;


	if (m_map)
	{
		free(m_map);
		free(m_pixels);
	}
	auto size = m_width * m_height * sizeof(BlockStruct);
	m_map = (BlockStruct*)malloc(m_width * m_height * sizeof(BlockStruct));
	m_pixels = (Pix*)malloc(m_width * m_height * sizeof(Pix));
	//if (m_noise)
	{
		//delete m_noise;
	//	delete m_noise_2;
	}
	//m_noise = new OpenSimplexNoise(m_seed);
	//m_noise_2 = new OpenSimplexNoise(m_seed * 123456);
	memset(m_map, 0, m_width * m_height * sizeof(BlockStruct));


	genHeightMap();
	genLayer0();
	genLayer1Grass();
}

void PriorGen::genHeightMap()
{
	m_height_map = new int[m_width];

	for (int x = 0; x < m_width; ++x)
		m_height_map[x] = m_terrain_level+getTerrainHeight(m_seed, x);
}

void PriorGen::genLayer0()
{
	for (int y = 0; y < m_height; ++y) {
		//ND_INFO("Generating line {}", y);
		for (int x = 0; x < m_width; ++x)
		{
			double yRatio = std::min((double)y / m_terrain_level, 1.0);
			auto& block = getBlock(x, y);


			int terrain = m_height_map[x];
			if (y > terrain) {
				block.block_id = BLOCK_AIR;
				block.setWall(WALL_AIR);
			}
			else if (y == terrain)
			{
				block.block_id = BLOCK_GRASS;
				//block.block_id = BLOCK_SNOW;
				block.setWall(WALL_AIR);
			}
			else if (y == terrain - 1) {
				block.block_id = BLOCK_DIRT;
				//block.block_id = BLOCK_SNOW;
				block.setWall(WALL_AIR);
			}
			else if (y > terrain - 8)
			{
				//5 block of dirt
				block.block_id = BLOCK_DIRT;
				//block.block_id = BLOCK_SNOW;
				block.setWall(WALL_DIRT);
			}
			else
			{
				block.setWall(WALL_STONE);
				//check if cave
				double ss = 1;

				double caveRegion = genOctave(0.5, x / m_noise_scale_factor_x, y / m_noise_scale_factor_y) * 1;

				double d = genOctave(1, x / m_noise_scale_factor_x, y / m_noise_scale_factor_y);

				d -= std::pow(caveRegion, 0.5);

				d += genOctave(2, x / m_noise_scale_factor_x, y / m_noise_scale_factor_y);
				d += genOctave(4, x / m_noise_scale_factor_x, y / m_noise_scale_factor_y);
				d += genOctave(8, x / m_noise_scale_factor_x, y / m_noise_scale_factor_y);


				if (d + m_noise_cutoff > std::clamp(yRatio, 0.4, 1.0))
				{
					//yes cave
					block.block_id = BLOCK_AIR;
				}
				else
				{
					//no cave
					if (genOre(0, x, y, m_ore_start, m_ore_start_full, m_ore_end_partial, m_ore_end, m_ore_clusterSize, m_ore_clusterNumber))
					{
						block.block_id = BLOCK_GOLD;
					}
					else
						block.block_id = BLOCK_STONE;
						//block.block_id = BLOCK_ICE;
				}
				continue;
			}
		}
	}

}

void PriorGen::genLayer1Grass()
{
	for (int x = 0; x < m_width; ++x)
	{
		auto& grass = getBlock(x, m_height_map[x]);
		if(m_height_map[x]>=m_height)
			continue;
		if(grass.block_id==BLOCK_GRASS)
		//if(grass.block_id==BLOCK_SNOW)
		{
			auto& upGrass = getBlock(x, m_height_map[x]+1);
			if(upGrass.isAir())
			{
				switch (std::rand() % 8)
				{
				case 0:
					upGrass.block_id = BLOCK_FLOWER;
					upGrass.block_metadata = std::rand() % 8;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					upGrass.block_id = BLOCK_GRASS_PLANT;
					upGrass.block_metadata = std::rand() % 4;
					break;
				case 7:
					break;
					//TreeGen::buildTree()
				}
			}


		}

	}
	
}

double PriorGen::genOctave(double octave, double x, double y)
{
	//double dd = m_noise_2->Evaluate(x * octave, y * octave);
	//dd = (dd + 1) / 2 / octave;
	//return dd;
	return 0;
}

bool PriorGen::genOre(double randomOffset, 
	double x, double y, 
	int startLevel,int startFullLevel, 
	int endPartialLevel, int endLevel,
	double clusterSize, double numberOfClusters)
{
	if (y < startLevel|| y > endLevel)
		return false;

	double cutoff=1;
	if(y<startFullLevel)
	{
		cutoff = (y - startLevel) / (startFullLevel - startLevel);
	}else if(y> endPartialLevel)
	{
		cutoff = 1-(y - endPartialLevel) / (endLevel - endPartialLevel);
	}


	double xx = (x + randomOffset) / m_noise_scale_factor_x* m_orescale;
	double yy = (y + randomOffset) / m_noise_scale_factor_y* m_orescale;


	double d = 0;

	d += genOctave(1, xx, yy)*m_ore_octave_magnitudes[0];
	d += genOctave(2, xx, yy)*m_ore_octave_magnitudes[1];
	d += genOctave(4, xx, yy)*m_ore_octave_magnitudes[2];
	d += genOctave(8, xx, yy)*m_ore_octave_magnitudes[3];

	cutoff /= m_ore_octave_magnitudes[4];

	return clusterSize+d<cutoff;
}


inline static PriorGen::Pix blockToColor(const BlockStruct& b)
{
	switch (b.block_id)
	{
	case BLOCK_AIR:
		return {255, 255, 255};
	case BLOCK_STONE:
		return {50, 50, 50};
	case BLOCK_ADAMANTITE:
		return {125, 0, 125};
	case BLOCK_GOLD:
		return {0, 255, 0};
	case BLOCK_TREE:
		return {50, 255, 50};
	case BLOCK_DIRT:
		return {125, 125, 0};
	case BLOCK_GRASS:
		return {0, 255, 0};
	case BLOCK_GRASS_PLANT:
	case BLOCK_FLOWER:
		return { 0, 255, 100 };
	}
	return {0, 0, 0};
}

void PriorGen::exportImage()
{
	Pix* image = (Pix*)malloc(m_width * m_height * sizeof(Pix));

	for (int j = 0; j < m_height; ++j)
		for (int i = 0; i < m_width; ++i)
		{
			int inverseY = m_height - j - 1;

			image[inverseY * m_width + i] = blockToColor(getBlock(i, j));
		}


	auto out = stbi_write_png((m_file_path + ".png").c_str(), m_width, m_height, STBI_rgb_alpha, image,
	                          m_width * sizeof(Pix));

	free(image);
	ND_INFO("Exported image with outpyr {}", out);
}

Texture* PriorGen::buildTexture()
{
	auto t = Texture::create(
		TextureInfo().size(m_width, m_height).filterMode(TextureFilterMode::NEAREST).format(TextureFormat::RGB));
	updateTexture(t);
	return t;
}

void PriorGen::updateTexture(Texture* t)
{
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			//auto yy = m_height - 1 - y;
			m_pixels[y * m_width + x] = blockToColor(m_map[y * m_width + x]);
		}
	}
	t->setPixels((uint8_t*)m_pixels);
}
