#include "ndpch.h"
#include "ThreadedChunkProvider.h"


void ThreadedChunkProvider::assignChunkLoad(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	uint64_t chunkOffsetN = chunkOffset;
	assignWork({ jobAssigment,chunk,chunkOffsetN,0, WorldIOAssignment::CHUNK_LOAD });
}

void ThreadedChunkProvider::assignChunkSave(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	uint64_t chunkOffsetN = chunkOffset;
	assignWork({ jobAssigment,chunk,chunkOffsetN,0,WorldIOAssignment::CHUNK_SAVE });
}
void ThreadedChunkProvider::assignWorldInfoSave(JobAssignment* jobAssigment, const WorldInfo* info)
{
	assignWork({ jobAssigment,(Chunk*)info,0,0,WorldIOAssignment::WORLD_META_SAVE });
}

void ThreadedChunkProvider::assignWorldInfoLoad(JobAssignment* jobAssigment, WorldInfo* info)
{
	assignWork({ jobAssigment,(Chunk*)info,0,0,WorldIOAssignment::WORLD_META_LOAD });
}

void ThreadedChunkProvider::assignNBTLoad(JobAssignment* jobAssigment,int chunkId, NBT* nbt)
{
	uint64_t chunkIdN = chunkId;

	assignWork({ jobAssigment,nbt,chunkIdN,0,WorldIOAssignment::NBT_READ });
}

void ThreadedChunkProvider::assignNBTSave(JobAssignment* jobAssigment,int chunkId, const NBT* nbt)
{
	uint64_t chunkIdN = chunkId;
	assignWork({ jobAssigment, const_cast<NBT*>(nbt),chunkIdN,0,WorldIOAssignment::NBT_WRITE });
}

void ThreadedChunkProvider::assignEntityLoad(JobAssignment* jobAssigment,int chunkId, WorldEntity*** entities, int* numberOfEntities)
{
	uint64_t chunkIdN = chunkId;
	uint64_t sizeP = reinterpret_cast<uint64_t>(numberOfEntities);
	void* arrayP = (void*)entities;
	assignWork({ jobAssigment, arrayP,chunkIdN,sizeP,WorldIOAssignment::ENTITY_READ });
}

void ThreadedChunkProvider::assignEntitySave(JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)
{
	uint64_t chunkIdN = chunkId;
	uint64_t sizeN = numberOfEntities;
	void* arrayP = (void*)entities;
	assignWork({ jobAssigment, arrayP,chunkIdN,sizeN,WorldIOAssignment::ENTITY_WRITE });
}
