#include "ndpch.h"
#include "FileChunkProvider.h"
#include "entity/EntityRegistry.h"

FileChunkProvider::FileChunkProvider(const std::string& file_path)
	: m_file_path(file_path),
	  m_nbt_saver(file_path + ".entity")
{
	m_nbt_saver.init();
}

void FileChunkProvider::proccessAssignments(std::vector<WorldIOAssignment>& assignments)
{
	auto stream = WorldIO::Session(m_file_path, true);
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
			if (!m_nbt_saver.isOpened())
				m_nbt_saver.beginSession();

			if(m_nbt_saver.setReadChunkID(assignment.chunkID))
				assignment.nbt->deserialize(&m_nbt_saver);
			//assignment.job->m_variable = 1;//mark as fucked up

			break;
		case WorldIOAssignment::NBT_WRITE:
			if (!m_nbt_saver.isOpened())
				m_nbt_saver.beginSession();
			m_nbt_saver.setWriteChunkID(assignment.chunkID);
			assignment.nbt->serialize(&m_nbt_saver);
			m_nbt_saver.flushWrite();
			break;
		case WorldIOAssignment::ENTITY_WRITE:
			{
				if (!m_nbt_saver.isOpened())
					m_nbt_saver.beginSession();
				m_nbt_saver.setWriteChunkID(assignment.chunkID);
				NBT t;
				t.set("entity_count", (int)assignment.entitySize);
				for (int i = 0; i < assignment.entitySize; ++i)
				{
					auto entity = assignment.entities[i];
					auto& entityNBT = t.get("ent_" + std::to_string(i), NBT());
					//todo building and coppying nbt = slow as f
					entity->save(entityNBT);
				}
				t.serialize(&m_nbt_saver);
				m_nbt_saver.flushWrite();
			}
			break;
		case WorldIOAssignment::ENTITY_READ:
			{
				if (!m_nbt_saver.isOpened())
					m_nbt_saver.beginSession();
				int number = 0;
				if (m_nbt_saver.setReadChunkID(assignment.chunkID)) {
					NBT t;
					t.deserialize(&m_nbt_saver);
					number = t.get("entity_count", 0);
					*assignment.entitySizePointer = number;
					if (number)
					{
						int index = 0;
						auto entities = new WorldEntity*[number];
						for (int i = 0; i < number; ++i)
						{
							if (!t.exists<NBT>("ent_" + std::to_string(i))) {
								ND_WARN("World chunk corruption detected,cannot load, entity is missing");
								//	throw "World chunk corruption detected,cannot load, entity is missing";
								entities[i] = nullptr;
								continue;
							}
							entities[i] = EntityRegistry::get()
								.loadInstance(t.get<NBT>("ent_" + std::to_string(i)));
						}
						*assignment.entitiesPointer = entities; //set array pointer to foreign(boss) variable
					}
				}else 	
					*assignment.entitySizePointer = number;
			}
			break;
		default:
			ASSERT(false, "invalid enum");
			break;
		}
		assignment.job->markDone();
	}
	if (m_nbt_saver.isOpened())
		m_nbt_saver.endSession();
}
