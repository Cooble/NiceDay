#pragma once
#include "IChunkProvider.h"

struct WorldIOAssignment
{
	nd::JobAssignment* job;

	union //pointer
	{
		Chunk* chunk;
		WorldInfo* worldInfo;
		NDUtils::Bitset* bool_gen;
		nd::NBT* nbt;
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

	nd::IBinaryStream::RWFunc func;

	WorldIOAssignment() = default;

	WorldIOAssignment(nd::JobAssignment* job, void* pointer, uint64_t data, uint64_t data2, int type)
		: job(job),
		  chunk((Chunk*)pointer),
		  data(data),
		  data2(data2)
	{
		*(int*)&this->type = type;
	}

	WorldIOAssignment(nd::JobAssignment* job, void* pointer, uint64_t data, uint64_t data2, int type,
	                  nd::IBinaryStream::RWFunc func)
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
class ThreadedChunkProvider : public nd::Worker<WorldIOAssignment>, public IChunkProvider
{
public:
	virtual ~ThreadedChunkProvider() = default;

	void assignChunkLoad(nd::JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignChunkSave(nd::JobAssignment* jobAssigment, Chunk* chunk, int chunkOffset) override;
	void assignWorldInfoSave(nd::JobAssignment* jobAssigment, const WorldInfo* info) override;
	void assignWorldInfoLoad(nd::JobAssignment* jobAssigment, WorldInfo* info) override;
	void assignNBTLoad(nd::JobAssignment* jobAssigment, int chunkId, nd::NBT* nbt) override;
	void assignNBTSave(nd::JobAssignment* jobAssigment, int chunkId, const nd::NBT* nbt) override;
	void assignEntityLoad(nd::JobAssignment* jobAssigment, int chunkId, WorldEntity*** entities, int* numberOfEntities)override;
	void assignEntitySave(nd::JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)override;
	void assignWait(nd::JobAssignment* jobAssigment) override;
	void assignBoolGenLoad(nd::JobAssignment* jobAssigment, NDUtils::Bitset* bitset) override;
	void assignBoolGenSave(nd::JobAssignment* jobAssigment, const NDUtils::Bitset* bitset) override;
	void assignSerialize(nd::JobAssignment* jobAssigment, int chunkId, const nd::IBinaryStream::RWFunc& func) override;
	void assignDeserialize(nd::JobAssignment* jobAssigment, int chunkId, const nd::IBinaryStream::RWFunc& func) override;
};
