#pragma once
#include "ndpch.h"
#include "core/IBinaryStream.h"
#include "files/DynamicSaver.h"

/*
WorldIO::Session is responsible for loading and saving world related data i.e. WorldInfo, Chunks.

Possible errors you might do:

	1. read when write_only mode is set
	2. using write_only when not writing the whole file at once
	3. bad filepath

*/

class World;
class Chunk;
struct WorldInfo;

namespace WorldIO
{

	class Session
	{
	public:
		Session(const std::string& path, bool write_mode, bool write_only = false);
		~Session();
	private:
		std::fstream* m_stream;
		std::string m_file_path;

#ifdef ND_DEBUG
		bool m_write_only;
		bool m_write_mode;
#endif


	public:
		void genWorldFile(const WorldInfo* info);

		void saveWorldMetadata(const WorldInfo* world);

		void saveGenBoolMap(const NDUtil::Bitset* bitset);
		void loadGenBoolMap(NDUtil::Bitset* bitset);

		//return true if success
		bool loadWorldMetadata(WorldInfo* world);

		void loadChunk(Chunk* chunk, int offset);

		void saveChunk(const Chunk* chunk, int offset);

		void close();
	};

	struct ChunkSegmentHeader
	{
		size_t next_index = std::numeric_limits<size_t>::max();

		inline bool hasNext() const { return next_index != std::numeric_limits<size_t>::max(); }
	};

	constexpr uint32_t DYNAMIC_SAVER_FREE_SEGMENTS = 10000;
	constexpr uint32_t DYNAMIC_SAVER_CHUNK_ID_COUNT = 10000;
	constexpr uint32_t DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE = 2000 + sizeof(ChunkSegmentHeader);


	IBinaryStream::RWStream streamFuncs(DynamicSaver* saver); 

}
