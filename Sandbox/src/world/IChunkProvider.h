#pragma once
#include "World.h"

class IChunkProvider
{
public:
	virtual ~IChunkProvider() =default;

	virtual void assignChunkLoad(JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignChunkSave(JobAssignment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignWorldInfoSave(JobAssignment* jobAssigment, const WorldInfo* info)=0;

	virtual void assignWorldInfoLoad(JobAssignment* jobAssigment, WorldInfo* info)=0;
	
	virtual void assignNBTLoad(JobAssignment* jobAssigment, int chunkId, NBT* nbt)=0;

	virtual void assignNBTSave(JobAssignment* jobAssigment, int chunkId, const NBT* nbt)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntityLoad(JobAssignment* jobAssigment, int chunkId, WorldEntity*** entities, int* numberOfEntities)=0;

	//entity array pointer passed in jobAssignment
	virtual void assignEntitySave(JobAssignment* jobAssigment, int chunkId, WorldEntity** entities, int numberOfEntities)=0;
	


};
