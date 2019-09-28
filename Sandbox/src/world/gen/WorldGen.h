#pragma once
class BlockAccess;
class Chunk;
class World;

class PriorGen;
class WorldGenLayer
{
protected:
	int m_chunk_radius_required=0;
public:
	WorldGenLayer() = default;
	virtual ~WorldGenLayer()=default;
	inline int getRequiredChunkRadius() const { return m_chunk_radius_required; }
	virtual void gen(World& w, Chunk& centerChunk) = 0;
};

class WorldGen
{
private:
	std::vector<WorldGenLayer*> m_layers;
	PriorGen* m_prior_gen = nullptr;

public:
	inline void registerLayer(WorldGenLayer* p) { m_layers.push_back(p); }
	float getTerrainHeight(int seed, float x);
	void genLayer0(World& w,Chunk& centerChunk);
};
