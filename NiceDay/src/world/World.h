#pragma once

struct BlockStruct {
	int id;
	int metadata;
	BlockStruct(int idd = 0, int metadataa = 0) :
		id(idd), metadata(metadataa)
	{}
};
class World
{
private:
	BlockStruct* m_blocks;
	int m_width, m_height;
	int m_size;
	void genWorld();

public:
	World();
	~World();
};

