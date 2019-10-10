#pragma once
#include "IChunkProvider.h"

struct WorldIOAssignment
{
	JobAssignment* job;

	union //pointer
	{
		Chunk* chunk;
		WorldInfo* worldInfo;
		NDUtil::Bitset* bool_gen;
		NBT* nbt;
		WorldEntity** entities;
		WorldEntity*** entitiesPointer;
	};

	union //data
	{
		uint64_t data;
		uint64_t chunkOffset;
		uint64_t chunkID;
	};

	union //data2
	{
		uint64_t data2;
		int* entitySizePointer;
		uint64_t entitySize;
	};

	enum : int
	{
		CHUNK_SAVE,
		CHUNK_LOAD,
		WORLD_META_SAVE,
		WORLD_META_LOAD,
		NBT_READ,
		NBT_WRITE,
		SERIALIZE,
		DESERIALIZE,
		ENTITY_READ,
		ENTITY_WRITE,
		BOOL_GEN_SAVE,
		BOOL_GEN_LOAD,
		WAIT,
	} type;

	IChunkProvider::SerialFunction func;

	WorldIOAssignment() = default;

	WorldIOAssignment(JobAssignment* job, void* pointer, uint64_t data, uint64_t data2, int type)
		: job(job),
		  chunk((Chunk*)pointer),
		  data(data),
		  data2(data2)
	{
		*(int*)&this->type = type;
	}

	WorldIOAssignment(JobAssignment* job, void* pointer, uint64_t data, uint64_t data2, int type,
	                  IChunkProvider::SerialFunction func)
		: job(job),
		  chunk((Chunk*)pointer),
		  data(data),
		  data2(data2),
		  func(func)
	{
		*(int*)&this->type = type;
	}
};

// File threaded chunk provider
class ThreadedChunkProvider : public Worker<WorldIOAssignment>, public IChunkProvider
{
public:
	virtual ~ThreadedChunkProvider() = default;

	void assignChunkLoad(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignChunkSave(JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignWorldInfoSave(JobAssignment* jobAssigment, const WorldInfo* info) override;
	void assignWorldInfoLoad(JobAssignment* jobAssigment, WorldInfo* info) override;
	void assignNBTLoad(JobAssignment* jobAssigment, int chunkId, NBT* nbt) override;
	void assignNBTSave(JobAssignment* jobAssigment, int chunkId, const NBT* nbt) override;
	void assignEntityLoad(JobAssignment* jobAssigment, int chunkId, WorldEntity*** entities, int* numberOfEntities)
	override;
	void assignEntitySave(JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)
	override;
	void assignWait(JobAssignment* jobAssigment) override;
	void assignBoolGenLoad(JobAssignment* jobAssigment, NDUtil::Bitset* bitset) override;
	void assignBoolGenSave(JobAssignment* jobAssigment, const NDUtil::Bitset* bitset) override;
	void assignSerialize(JobAssignment* jobAssigment, int chunkId, SerialFunction func) override;
	void assignDeserialize(JobAssignment* jobAssigment, int chunkId, SerialFunction func) override;
};
