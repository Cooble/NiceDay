#pragma once
#include "ChunkPack.h"
class Chunk;
class World;

struct WorldGenAssignment
{
	nd::JobAssignment* job;

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

	WorldGenAssignment(nd::JobAssignment* job, int chunkId, Chunk* chunk) :
		job(job),
		chunk_id(chunkId),
		chunk(chunk)
	{
		type = GENERATION;
	}

	WorldGenAssignment(nd::JobAssignment* job, const ChunkPack& pack, int bitBounds) :
		job(job),
		chunkPack(pack),
		bitBounds(bitBounds)
	{
		type = BOUND_UPDATE;
	}
};


class ThreadedWorldGen : public nd::Worker<WorldGenAssignment>
{
private:
	World* m_world;
public:
	explicit ThreadedWorldGen(World* w): m_world(w)
	{
	}

	virtual ~ThreadedWorldGen() = default;

	void proccessAssignments(std::vector<WorldGenAssignment>& assignments) override;

	void assignChunkGen(nd::JobAssignment* job, Chunk* chunk);
	void assignChunkBoundUpdate(nd::JobAssignment* job, const ChunkPack& pack, int bitBounds);
	void assignWait(nd::JobAssignmentP job);
};
