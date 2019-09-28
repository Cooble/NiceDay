#pragma once
#include "BlockAccess.h"

// contains a slice of world (chunkPointers)
// 
class ChunkPack:public BlockAccess
{
private:
	half_int m_chunk_pos;
	std::array<Chunk*,9> m_chunks;
public:
	ChunkPack() = default;
	ChunkPack(half_int centerPos,std::initializer_list<Chunk*> chunks);
	virtual ~ChunkPack()=default;
	BlockStruct* getBlockM(int x, int y) override;
	void setBlock(int x, int y, BlockStruct& block) override;
	void setBlockWithNotify(int x, int y, BlockStruct& block) override;
	void setWall(int x, int y, int wall_id) override;
	Chunk* getChunkM(int cx,int cy)override;
	uint8_t* getLightLevel(int x, int y);

	auto begin()
	{
		return m_chunks.begin();
	}
	auto end()
	{
		return m_chunks.end();
	}

	inline half_int getCenterChunk()const { return m_chunk_pos; }

	void markLightJobDone();
	void assignLightJob();

	static ChunkPack fromChunk(Chunk* c);
};

