#pragma once
#include "ChunkPack.h"
class Chunk;
class JobAssignment;
class World;

struct WorldGenAssignment
{
	JobAssignment* job;

	enum
	{
		GENERATION,
		BOUND_UPDATE,
		WAIT
	} type;

	int chunk_id;
	Chunk* chunk;

	ChunkPack chunkPack;
	int bitBounds;
	WorldGenAssignment() = default;

	WorldGenAssignment(JobAssignment* job, int chunkId, Chunk* chunk) :
		job(job),
		chunk_id(chunkId),
		chunk(chunk)
	{
		type = GENERATION;
	}

	WorldGenAssignment(JobAssignment* job, const ChunkPack& pack, int bitBounds) :
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

	void assignChunkGen(JobAssignment* job, Chunk* chunk);
	void assignChunkBoundUpdate(JobAssignment* job, const ChunkPack& pack, int bitBounds);
	void assignWait(JobAssignmentP job);
};
