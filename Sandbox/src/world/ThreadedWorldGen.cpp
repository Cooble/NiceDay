#include "ndpch.h"
#include "ThreadedWorldGen.h"
#include "World.h"

void ThreadedWorldGen::proccessAssignments(std::vector<WorldGenAssignment>& assignments)
{
	for (auto & assignment : assignments)
	{
		if (assignment.type == WorldGenAssignment::GENERATION) {
			//ND_INFO("generating chunk {}, {}",half_int::X(assignment.chunk_id), half_int::Y(assignment.chunk_id));
			auto& gen = m_world->getWorldGen();
			gen.genLayer0(*m_world, *assignment.chunk);
			//ND_INFO("done chunk {}, {}", half_int::X(assignment.chunk_id), half_int::Y(assignment.chunk_id));
			//ND_INFO("from current Assignment the is still {} left",assignment.job->m_main- assignment.job->m_worker);
		}
		else if(assignment.type == WorldGenAssignment::BOUND_UPDATE)
		{
			auto center = assignment.chunkPack.getCenterChunk();
			World::updateChunkBounds(assignment.chunkPack, center.x, center.y, assignment.bitBounds);
		}
		assignment.job->markDone();
	}
}

void ThreadedWorldGen::assignChunkGen(JobAssignment* job, Chunk* chunk)
{
	WorldGenAssignment as(job, chunk->chunkID(), chunk);

	assignWork(as);
}

void ThreadedWorldGen::assignChunkBoundUpdate(JobAssignment* job, const ChunkPack& pack,int bitBounds)
{
	WorldGenAssignment as(job, pack,bitBounds);

	assignWork(as);
}
