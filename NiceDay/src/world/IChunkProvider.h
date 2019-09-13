#pragma once
#include "World.h"

class IChunkProvider
{
public:
	virtual ~IChunkProvider() =default;

	virtual void assignChunkLoad(JobAssigment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignChunkSave(JobAssigment* jobAssigment,Chunk* chunk, int chunkOffset)=0;

	virtual void assignWorldInfoSave(JobAssigment* jobAssigment, const WorldInfo* info)=0;

	virtual void assignWorldInfoLoad(JobAssigment* jobAssigment, WorldInfo* info)=0;
	
	virtual void assignNBTLoad(JobAssigment* jobAssigment, NBT* nbt)=0;
	
	virtual void assignNBTSave(JobAssigment* jobAssigment, const NBT* nbt)=0;


};
