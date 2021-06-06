﻿#include "ndpch.h"
#include "ThreadedChunkProvider.h"

using namespace nd;

void ThreadedChunkProvider::assignChunkLoad(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	jobAssigment->assign();
	uint64_t chunkOffsetN = chunkOffset;
	assignWork({ jobAssigment,chunk,chunkOffsetN,0, WorldIOAssignment::CHUNK_LOAD });
}

void ThreadedChunkProvider::assignChunkSave(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset)
{
	jobAssigment->assign();
	uint64_t chunkOffsetN = chunkOffset;
	assignWork({ jobAssigment,chunk,chunkOffsetN,0,WorldIOAssignment::CHUNK_SAVE });
}
void ThreadedChunkProvider::assignWorldInfoSave(JobAssignment* jobAssigment, const WorldInfo* info)
{
	jobAssigment->assign();
	assignWork({ jobAssigment,(Chunk*)info,0,0,WorldIOAssignment::WORLD_META_SAVE });
}

void ThreadedChunkProvider::assignWorldInfoLoad(JobAssignment* jobAssigment, WorldInfo* info)
{
	jobAssigment->assign();
	assignWork({ jobAssigment,(Chunk*)info,0,0,WorldIOAssignment::WORLD_META_LOAD });
}



void ThreadedChunkProvider::assignEntityLoad(JobAssignment* jobAssigment,int chunkId, WorldEntity*** entities, int* numberOfEntities)
{
	jobAssigment->assign();
	uint64_t chunkIdN = chunkId;
	uint64_t sizeP = reinterpret_cast<uint64_t>(numberOfEntities);
	void* arrayP = (void*)entities;
	assignWork({ jobAssigment, arrayP,chunkIdN,sizeP,WorldIOAssignment::ENTITY_READ });
}

void ThreadedChunkProvider::assignEntitySave(JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)
{
	jobAssigment->assign();
	uint64_t chunkIdN = chunkId;
	uint64_t sizeN = numberOfEntities;
	void* arrayP = (void*)entities;
	assignWork({ jobAssigment, arrayP,chunkIdN,sizeN,WorldIOAssignment::ENTITY_WRITE });
}

void ThreadedChunkProvider::assignWait(JobAssignment* jobAssigment)
{
	jobAssigment->assign();
	assignWork({ jobAssigment, nullptr,0,0,WorldIOAssignment::WAIT });
}

void ThreadedChunkProvider::assignBoolGenLoad(JobAssignment* jobAssigment, Utils::Bitset* bitset)
{
	jobAssigment->assign();
	assignWork({ jobAssigment, bitset,0,0,WorldIOAssignment::BOOL_GEN_LOAD });
}

void ThreadedChunkProvider::assignBoolGenSave(JobAssignment* jobAssigment, const Utils::Bitset* bitset)
{
	jobAssigment->assign();
	assignWork({ jobAssigment, const_cast<Utils::Bitset*>(bitset),0,0,WorldIOAssignment::BOOL_GEN_SAVE });
}

void ThreadedChunkProvider::assignSerialize(JobAssignment* jobAssigment, int chunkId,const IBinaryStream::RWFunc& func)
{
	jobAssigment->assign();
	assignWork({ jobAssigment, nullptr,(uint64_t)chunkId,0,WorldIOAssignment::SERIALIZE ,func});
}

void ThreadedChunkProvider::assignDeserialize(JobAssignment* jobAssigment, int chunkId, const IBinaryStream::RWFunc& func)
{
	jobAssigment->assign();
	assignWork({ jobAssigment, nullptr,(uint64_t)chunkId,0,WorldIOAssignment::DESERIALIZE ,func });
}

void ThreadedChunkProvider::assignNBTLoad(JobAssignment* jobAssigment, int chunkId, NBT* nbt)
{
	assignDeserialize(jobAssigment, chunkId, ND_IBS_HOOK(&NBT::read,nbt));
}

void ThreadedChunkProvider::assignNBTSave(JobAssignment* jobAssigment, int chunkId, const NBT* nbt)
{
	assignSerialize(jobAssigment, chunkId, ND_IBS_HOOK(&NBT::write, nbt));
}
