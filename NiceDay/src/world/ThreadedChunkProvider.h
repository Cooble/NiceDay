#pragma once
#include "IChunkProvider.h"

struct WorldIOAssignment
{
	JobAssigment* job;

	union//pointer
	{
		Chunk* chunk;
		WorldInfo* worldInfo;
		NBT* nbt;
	};
	union//data
	{
		int chunkOffset;
		int chunkID;
	};
	enum:int
	{
		CHUNK_SAVE,
		CHUNK_LOAD,
		WORLD_META_SAVE,
		WORLD_META_LOAD,
		NBT_READ,
		NBT_WRITE,
	} type;
	WorldIOAssignment(JobAssigment* job, void* pointer, int data, int type)
	:job(job),
	chunk((Chunk*)pointer),
	chunkOffset(data)
	{
		*(int*)&this->type = type;
	}
};
// File threaded chunk provider
class ThreadedChunkProvider:public Worker<WorldIOAssignment>,public IChunkProvider
{
public:
	virtual ~ThreadedChunkProvider()=default;

	void assignChunkLoad(JobAssigment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignChunkSave(JobAssigment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignWorldInfoSave(JobAssigment* jobAssigment, const WorldInfo* info) override;
	void assignWorldInfoLoad(JobAssigment* jobAssigment, WorldInfo* info) override;
	void assignNBTLoad(JobAssigment* jobAssigment, NBT* nbt) override;
	void assignNBTSave(JobAssigment* jobAssigment, const NBT* nbt) override;
};


