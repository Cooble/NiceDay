#include "ndpch.h"
#include "FileChunkProvider.h"

FileChunkProvider::FileChunkProvider(const std::string& file_path)
:m_file_path(file_path),
m_nbt_saver(file_path+".path")
{
	m_nbt_saver.init();
}

void FileChunkProvider::proccessAssignments(std::vector<WorldIOAssignment>& assignments)
{
	auto stream = WorldIO::Session(m_file_path, true);
	m_nbt_saver.beginSession();

	for (auto& assignment : assignments)
	{
		switch (assignment.type)
		{
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
			stream.loadWorldMetadata(assignment.worldInfo);
			break;
		case WorldIOAssignment::NBT_READ:
			m_nbt_saver.setReadChunkID(assignment.chunkID);
			assignment.nbt->deserialize(&m_nbt_saver);
			break;
		case WorldIOAssignment::NBT_WRITE:
			m_nbt_saver.setWriteChunkID(assignment.chunkID);
			assignment.nbt->serialize(&m_nbt_saver);
			m_nbt_saver.flushWrite();
			break;
		}
		assignment.job->markDone();
	}
	m_nbt_saver.endSession();
}
