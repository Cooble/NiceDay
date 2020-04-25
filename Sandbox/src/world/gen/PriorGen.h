#pragma once
#include "world/block/Block.h"
#include "graphics/API/Texture.h"

//class OpenSimplexNoise;

class PriorGen
{
public:
	struct Pix;
private:
	std::string m_file_path;
	int m_width, m_height;
	BlockStruct* m_map=nullptr;
	Pix* m_pixels=nullptr;
	uint64_t m_seed;
	//OpenSimplexNoise* m_noise;
	//OpenSimplexNoise* m_noise_2;
	int m_terrain_level;
	int* m_height_map;

public:
	double m_noise_scale_factor_x=30;
	double m_noise_scale_factor_y=20;
	double m_noise_cutoff=0.357;
	double m_time=0;

	double m_ore_clusterSize = 0;
	double m_ore_clusterNumber = 1;
	double m_ore_start = 0;
	double m_ore_start_full = 100;
	double m_ore_end_partial = 200;
	double m_ore_end = 300;
	double m_orescale=6.849;

	double m_ore_octave_magnitudes[5] = { 1.897,0,0,0,4.566 };

public:
	struct Pix
	{
		union{
		uint8_t src[3];
		struct
		{
			uint8_t r, g, b;
		};
		};
	};

	PriorGen(const std::string& filePath);
	~PriorGen();

	float getTerrainHeight(int seed, float x);
	void gen(uint64_t seed, int terrainLevel, int width, int height);
	void genHeightMap();
	void genLayer0();
	void genLayer1Grass();

	double genOctave(double octave, double x, double y);
	bool genOre(double randomOffset, double x, double y, int startLevel, int startFullLevel, int endLevel, int endPartialLevel, double
	            clusterSize, double numberOfClusters);

	inline BlockStruct& getBlock(int x, int y)
	{
		ASSERT(x >= 0 && x < m_width&&y >= 0 && y < m_height, "INvalid coords");
		return m_map[y*m_width + x];
	}

	void exportImage();
	Texture* buildTexture();
	void updateTexture(Texture* t);
};
