#include "ndpch.h"
#include "FileChunkProvider.h"
#include "entity/EntityRegistry.h"
#include "entity/EntityAllocator.h"

using namespace nd;

FileChunkProvider::FileChunkProvider(const std::string& file_path)
	: m_file_path(file_path),
	  m_nbt_saver(file_path + ".entity")
{
	m_nbt_saver.init();
}

void FileChunkProvider::proccessAssignments(std::vector<WorldIOAssignment>& assignments)
{
	auto stream = WorldIO::Session(m_file_path, true);
	IBinaryStream::RWStream streamFuncs = WorldIO::streamFuncs(&m_nbt_saver);

	for (auto& assignment : assignments)
	{
		if (assignment.job->isFailure()) {
			assignment.job->markDone();
			continue;
		}
		switch (assignment.type)
		{
		case WorldIOAssignment::WAIT:
			break;
		case WorldIOAssignment::BOOL_GEN_SAVE:
			stream.saveGenBoolMap(assignment.bool_gen);
			break;
		case WorldIOAssignment::BOOL_GEN_LOAD:
			stream.loadGenBoolMap(assignment.bool_gen);
			if (assignment.bool_gen->bitSize() == 0)
				assignment.job->m_variable = JobAssignment::JOB_FAILURE;
			break;
		case WorldIOAssignment::CHUNK_SAVE:
			stream.saveChunk(assignment.chunk, assignment.chunkOffset);
			break;
		case WorldIOAssignment::CHUNK_LOAD:
			stream.loadChunk(assignment.chunk, assignment.chunkOffset);
			break;
		case WorldIOAssignment::WORLD_META_SAVE:
			stream.saveWorldMetadata(assignment.worldInfo);
			break;
		case WorldIOAssignment::WORLD_META_LOAD:
			{
				bool success = stream.loadWorldMetadata(assignment.worldInfo);
				if (!success)
					assignment.job->m_variable = JobAssignment::JOB_FAILURE;
			}
			break;
		case WorldIOAssignment::DESERIALIZE:
			m_nbt_saver.beginSession();

			if (m_nbt_saver.setReadChunkID(assignment.chunkID))
				assignment.func(streamFuncs);
			else 
				assignment.job->m_variable = JobAssignment::JOB_FAILURE;//mark as fucked up
			break;
		case WorldIOAssignment::SERIALIZE:
			m_nbt_saver.beginSession();
			m_nbt_saver.setWriteChunkID(assignment.chunkID);
			assignment.func(streamFuncs);
			m_nbt_saver.flushWrite();
			break;
		case WorldIOAssignment::ENTITY_WRITE:
			{
				m_nbt_saver.beginSession();
				NBT t;
				m_nbt_saver.setWriteChunkID(assignment.chunkID);
				if (assignment.entitySize)
				{
					NBT list;
					for (int i = 0; i < assignment.entitySize; ++i)
					{
						auto entity = assignment.entities[i];
						NBT entityNBT;
						entity->save(entityNBT);
						list.push_back(std::move(entityNBT));
					}
					t["list"] = std::move(list);
					t.write(streamFuncs);
				}
				m_nbt_saver.flushWrite();
				if (m_nbt_saver.setReadChunkID(assignment.chunkID)) {
					NBT tt;
					tt.read(streamFuncs);
					ASSERT(tt == t, "Shi");
				}
				
			}
			break;
		case WorldIOAssignment::ENTITY_READ:
			{
				m_nbt_saver.beginSession();
				if (m_nbt_saver.setReadChunkID(assignment.chunkID))
				{
					NBT t;
					t.read(streamFuncs);
					auto& ls = t["list"];
					int number = ls.size();
					*assignment.entitySizePointer = number;
					if (number)
					{
						auto entities = new WorldEntity*[number];
						for (int i = 0; i < ls.size(); ++i)
							entities[i] = EntityAllocator::loadInstance(ls[i]);
						*assignment.entitiesPointer = entities; //set array pointer to foreign(boss) variable
					}
				}
				else
					*assignment.entitySizePointer = 0;
			}
			break;
		default:
			ASSERT(false, "invalid enum");
			break;
		}
		assignment.job->markDone();
	}	
	m_nbt_saver.endSession();
}
