#pragma once
#include "ChunkPack.h"
class Chunk;
class JobAssigment;
class World;

struct WorldGenAssignment
{
	JobAssigment* job;
	enum
	{
		GENERATION,
		BOUND_UPDATE
	}type;
	union
	{
		struct {
			int chunk_id;
			Chunk* chunk;
		};
		struct
		{
		ChunkPack chunkPack;
		int bitBounds;
		};
	};
	WorldGenAssignment(JobAssigment* job, int chunkId, Chunk* chunk) :
		job(job),
		chunk_id(chunkId),
		chunk(chunk)
	{
		type = GENERATION;
	}
	WorldGenAssignment(JobAssigment* job, const ChunkPack& pack, int bitBounds) :
		job(job),
		chunkPack(pack),
		bitBounds(bitBounds)
	{
		type = BOUND_UPDATE;
	}
};


class ThreadedWorldGen : public Worker<WorldGenAssignment>
{
private:
	World* m_world;
public:
	explicit ThreadedWorldGen(World* w): m_world(w)
	{
	}

	virtual ~ThreadedWorldGen() = default;

	void proccessAssignments(std::vector<WorldGenAssignment>& assignments) override;

	void assignChunkGen(JobAssigmentP job, Chunk* chunk);
	void assignChunkBoundUpdate(JobAssigmentP job, const ChunkPack& chunk,int bitBounds);
};
