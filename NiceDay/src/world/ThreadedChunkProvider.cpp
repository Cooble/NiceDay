#include "ndpch.h"
#include "ThreadedChunkProvider.h"


void ThreadedChunkProvider::assignChunkLoad(JobAssigment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	assignWork({ jobAssigment,chunk,chunkOffset, WorldIOAssignment::CHUNK_LOAD });
}

void ThreadedChunkProvider::assignChunkSave(JobAssigment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	assignWork({ jobAssigment,chunk,chunkOffset,WorldIOAssignment::CHUNK_SAVE });
}
void ThreadedChunkProvider::assignWorldInfoSave(JobAssigment* jobAssigment, const WorldInfo* info)
{
	assignWork({ jobAssigment,(Chunk*)info,0,WorldIOAssignment::WORLD_META_SAVE });
}

void ThreadedChunkProvider::assignWorldInfoLoad(JobAssigment* jobAssigment, WorldInfo* info)
{
	assignWork({ jobAssigment,(Chunk*)info,0,WorldIOAssignment::WORLD_META_LOAD });
}

void ThreadedChunkProvider::assignNBTLoad(JobAssigment* jobAssigment, NBT* nbt)
{
	assignWork({ jobAssigment,nbt,0,WorldIOAssignment::NBT_READ });
}

void ThreadedChunkProvider::assignNBTSave(JobAssigment* jobAssigment, const NBT* nbt)
{
	assignWork({ jobAssigment, const_cast<NBT*>(nbt),0,WorldIOAssignment::NBT_WRITE });
}
