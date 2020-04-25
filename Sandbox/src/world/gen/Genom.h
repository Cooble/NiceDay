#pragma once
#include "world/block/Block.h"
#include "graphics/API/Texture.h"



class OpenSimplexNoise;
class Genom
{
public:
	struct Pix
	{
		union {
			uint8_t src[3];
			struct
			{
				uint8_t r, g, b;
			};
		};
	};
	struct Layer
	{
		enum
		{
			SIMPLEX,
			CUTOFF,
			MULTI
		}type;
		int seed=0;
		double cutoff;
		double frequency;
		double multi;
		bool enabled=true;
	};
public:
	int m_width=10, m_height=10;
	BlockID* m_blocks = nullptr;
	double* m_map= nullptr;
	Pix* m_pixels = nullptr;
	uint64_t m_seed=-1;
	OpenSimplexNoise* m_noise;
	int m_terrain_level;
	int* m_height_map;
	std::vector<Layer> m_layers;

	void updateNBT(const NBT& nbt);
	void gen();
	Texture* buildTexture();
	void updateTexture(Texture* t);
	inline BlockID& getBlock(int x, int y)
	{
		//ASSERT(x >= 0 && x < m_width&& y >= 0 && y < m_height, "INvalid coords");
		return m_blocks[y * m_width + x];
	}
	inline double& getDouble(int x, int y)
	{
		//ASSERT(x >= 0 && x < m_width&& y >= 0 && y < m_height, "INvalid coords");
		return m_map[y * m_width + x];
	}
};
