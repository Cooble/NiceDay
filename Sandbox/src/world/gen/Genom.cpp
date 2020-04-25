#include "Genom.h"
#include "OpenSimplexNoise.hpp"
#include "graphics/API/Texture.h"
#include "world/block/block_datas.h"

inline static Genom::Pix blockToColor(const BlockStruct& b)
{
	switch (b.block_id)
	{
	case BLOCK_AIR:
		return { 255, 255, 255 };
	case BLOCK_STONE:
		return { 50, 50, 50 };
	case BLOCK_ADAMANTITE:
		return { 125, 0, 125 };
	case BLOCK_GOLD:
		return { 0, 255, 0 };
	case BLOCK_TREE:
		return { 50, 255, 50 };
	case BLOCK_DIRT:
		return { 125, 125, 0 };
	case BLOCK_GRASS:
		return { 0, 255, 0 };
	case BLOCK_GRASS_PLANT:
	case BLOCK_FLOWER:
		return { 0, 255, 100 };
	}
	return { 0, 0, 0 };
}

inline static Genom::Pix doubleToColor(double b)
{
	auto n= (uint8_t)((b + 1) /2 *255);
	return { n, n, n };
}
static NBT oldNBT;
static bool wasChange = true;
void Genom::updateNBT(const NBT& nbt)
{
	//ignore if no change
	if (oldNBT == nbt)
		return;
	oldNBT = nbt;
	wasChange = true;
	
	if ((int)nbt["width"] != m_width || (int)nbt["height"] != m_height)
	{
		m_width = nbt["width"];
		m_height = nbt["height"];
		if (m_blocks)
			free(m_blocks);
		m_blocks = (BlockID*)malloc(m_width * m_height * sizeof(BlockID));
		if (m_pixels)
			free(m_pixels);
		m_pixels = (Pix*)malloc(m_width * m_height * sizeof(Pix));
		if (m_map)
			free(m_map);
		m_map = (double*)malloc(m_width * m_height * sizeof(double));
	}
	if(m_seed!=(uint64_t)nbt["seed"])
	{
		m_seed = nbt["seed"];
		if (m_noise)
			delete m_noise;
		m_noise = new OpenSimplexNoise(m_seed);
	}
	
	m_layers.clear();
	for (auto& layer : nbt["layers"].arrays())
	{
		Layer l;
		if(layer.exists("cutoff"))
		{
			l.type = Layer::CUTOFF;
			l.cutoff = layer["cutoff"];
		}
		else if (layer.exists("freq"))
		{
			l.type = Layer::SIMPLEX;
			l.frequency = layer["freq"];
			if (l.frequency == 0)
				l.frequency = 1;
			layer.load("multi", l.multi, 1.0);
			layer.load("seed", l.seed, 0);
		}
		else if (layer.exists("multi"))
		{
			l.type = Layer::MULTI;
			l.multi = layer["multi"];
		}
		layer.load("enabled",l.enabled,true);
		m_layers.push_back(l);
	}
	gen();
}

void Genom::gen()
{
	ZeroMemory(m_map, sizeof(double) * m_width * m_height);
	for (auto& layer : m_layers)
	{
		if(!layer.enabled)
			continue;
		if (layer.type == Layer::SIMPLEX) {
			for (int x = 0; x < m_width; ++x)
				for (int y = 0; y < m_height; ++y)
					getDouble(x, y) += m_noise->Evaluate(x / layer.frequency+layer.seed*100000, y / layer.frequency + layer.seed * 100000) * layer.multi;
		}
		else if (layer.type == Layer::CUTOFF) {
			if (layer.cutoff > 0)
				for (int x = 0; x < m_width; ++x)
					for (int y = 0; y < m_height; ++y) {
						auto& d = getDouble(x, y);
						d = d < layer.cutoff ? 0 : d;
					}
			else
			{
				for (int x = 0; x < m_width; ++x)
					for (int y = 0; y < m_height; ++y) {
						auto& d = getDouble(x, y);
						d = d > layer.cutoff ? 1 : d;
					}
			}
		}
		else if (layer.type == Layer::MULTI) {
			for (int x = 0; x < m_width; ++x)
				for (int y = 0; y < m_height; ++y)
					getDouble(x, y)*=layer.multi;
		
		}
	
	}
	for (int i = 0; i < m_width * m_height; ++i)
		m_pixels[i] = doubleToColor(m_map[i]);
}

Texture* Genom::buildTexture()
{
	auto t = Texture::create(
		TextureInfo().size(m_width, m_height).filterMode(TextureFilterMode::NEAREST).format(TextureFormat::RGB));
	updateTexture(t);
	return t;
}

void Genom::updateTexture(Texture* t)
{
	if (wasChange) {
		wasChange = false;
		t->setPixels((uint8_t*)m_pixels);
	}
}
