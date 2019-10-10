#pragma once
#include "World.h"

class IChunkProvider
{
public:
	typedef std::function<void(IStream*)> SerialFunction;

	virtual ~IChunkProvider() =default;

	virtual void assignChunkLoad(JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignChunkSave(JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignWorldInfoSave(JobAssignment* jobAssigment, const WorldInfo* info)=0;

	virtual void assignWorldInfoLoad(JobAssignment* jobAssigment, WorldInfo* info)=0;

	virtual void assignBoolGenSave(JobAssignment* jobAssigment, const NDUtil::Bitset* bitset) = 0;

	virtual void assignBoolGenLoad(JobAssignment* jobAssigment, NDUtil::Bitset* bitset) = 0;
	
	virtual void assignNBTLoad(JobAssignment* jobAssigment, int chunkId, NBT* nbt)=0;

	virtual void assignNBTSave(JobAssignment* jobAssigment, int chunkId, const NBT* nbt)=0;
	
	virtual void assignSerialize(JobAssignment* jobAssigment, int chunkId, SerialFunction func)=0;

	virtual void assignDeserialize(JobAssignment* jobAssigment, int chunkId, SerialFunction func)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntityLoad(JobAssignment* jobAssigment, int chunkId, WorldEntity*** entities, int* numberOfEntities)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntitySave(JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)=0;
	
	// after all async work prior to this call job will be marked done
	// used to wait for all prior assignments to  be done
	virtual void assignWait(JobAssignment* jobAssigment) = 0;


	


};
