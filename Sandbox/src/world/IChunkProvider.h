#pragma once
#include "World.h"

class IChunkProvider
{
public:

	virtual ~IChunkProvider() =default;

	virtual void assignChunkLoad(nd::JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignChunkSave(nd::JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignWorldInfoSave(nd::JobAssignment* jobAssigment, const WorldInfo* info)=0;

	virtual void assignWorldInfoLoad(nd::JobAssignment* jobAssigment, WorldInfo* info)=0;

	virtual void assignBoolGenSave(nd::JobAssignment* jobAssigment, const NDUtils::Bitset* bitset) = 0;

	virtual void assignBoolGenLoad(nd::JobAssignment* jobAssigment, NDUtils::Bitset* bitset) = 0;
	
	virtual void assignNBTLoad(nd::JobAssignment* jobAssigment, int chunkId, nd::NBT* nbt)=0;

	virtual void assignNBTSave(nd::JobAssignment* jobAssigment, int chunkId, const nd::NBT* nbt)=0;
	
	virtual void assignSerialize(nd::JobAssignment* jobAssigment, int chunkId, const nd::IBinaryStream::RWFunc& func)=0;

	virtual void assignDeserialize(nd::JobAssignment* jobAssigment, int chunkId, const nd::IBinaryStream::RWFunc& func)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntityLoad(nd::JobAssignment* jobAssigment, int chunkId, WorldEntity*** entities, int* numberOfEntities)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntitySave(nd::JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)=0;
	
	// after all async work prior to this call job will be marked done
	// used to wait for all prior assignments to  be done
	virtual void assignWait(nd::JobAssignment* jobAssigment) = 0;


	


};
