#pragma once
class Chunk;
class World;

class WorldGenLayer
{
	
};

class WorldGen
{
private:

public:
	float getTerrainHeight(int seed, float x);
	void gen(int seed, World* w, Chunk& c);

	
};
