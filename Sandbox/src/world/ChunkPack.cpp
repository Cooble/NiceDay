#include "ndpch.h"
#include "ChunkPack.h"
#include "World.h"




ChunkPack::ChunkPack(half_int chunkCenterPos, std::initializer_list<Chunk*> chunks):
m_chunk_pos(chunkCenterPos)
{
	ASSERT(chunks.size()==9,"ChunkPack requires 9 chunkpointer entries (can be nullptr)")
	int index = 0;
	for (int i = 0; i < 9; ++i)
	{
		m_chunks[i] = *(chunks.begin() + i);
	}
}

BlockStruct* ChunkPack::getBlockM(int x, int y)
{
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;
	auto chunk = getChunkM(cx, cy);
	if (chunk == nullptr)
		return nullptr;

	return &chunk->block(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
}

void ChunkPack::setBlock(int x, int y, BlockStruct& newBlock)
{
	auto oldBlock = getBlockM(x, y);
	if (oldBlock == nullptr)
		return;

	memcpy(newBlock.wall_id, oldBlock->wall_id, sizeof(newBlock.wall_id));
	memcpy(newBlock.wall_corner, oldBlock->wall_corner, sizeof(newBlock.wall_corner));

	*oldBlock = newBlock;
}

void ChunkPack::setBlockWithNotify(int x, int y, BlockStruct& newBlock)
{
	ASSERT(false, "setblockwith notify doesn't work for chunkpack");
}

void ChunkPack::setWall(int x, int y, int wall_id)
{
	auto oldBlock = getBlockM(x, y);
	if (oldBlock == nullptr)
		return;
	//set block from old block acordingly
	auto& blok = *oldBlock;

	if (!blok.isWallOccupied() && wall_id == 0) //cannot erase other blocks parts on this shared block
		return;
	blok.setWall(wall_id);
}

Chunk* ChunkPack::getChunkM(int cx, int cy)
{
	cx = cx - (m_chunk_pos.x-1);
	cy = cy - (m_chunk_pos.y-1);
	if (cx >= 0 && cx < 3 && cy >= 0 && cy < 3)
		return m_chunks[cy * 3 + cx];
	return nullptr;
}

uint8_t* ChunkPack::getLightLevel(int x, int y)
{
	int cx = x >> WORLD_CHUNK_BIT_SIZE;
	int cy = y >> WORLD_CHUNK_BIT_SIZE;
	auto chunk = getChunkM(cx, cy);
	if (chunk == nullptr)
		return nullptr;

	return &chunk->lightLevel(x & (WORLD_CHUNK_SIZE - 1), y & (WORLD_CHUNK_SIZE - 1));
}

void ChunkPack::markLightJobDone()
{
	for (int i = 0; i < 9; ++i)
	{
		auto c = m_chunks[i];
		if (c)
			c->getLightJob().markDone();
	}
}

void ChunkPack::assignLightJob()
{
	for (int i = 0; i < 9; ++i)
	{
		auto c = m_chunks[i];
		if (c)
			c->getLightJob().assign();
	}
}

ChunkPack ChunkPack::fromChunk(Chunk* c)
{
	ChunkPack out;
	out.m_chunk_pos = c->chunkID();
	memset(out.m_chunks.data(), 0, sizeof(out.m_chunks));
	out.m_chunks[4] = c;
	return out;
}
