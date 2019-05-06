#pragma once
#include "ndpch.h"

/*
WorldIO::Session is responsible for loading and saving world related data i.e. WorldInfo, Chunks.

Possible errors you might do:

	1. read when write_only mode is set
	2. using write_only when not writing the whole file at once
	3. bad filepath

*/

class World;
class Chunk;

struct WorldIOInfo {
	std::string world_name;
	int chunk_width, chunk_height;
	long seed;

};
namespace WorldIO {
	class Session {
	public:
		Session(const std::string& path, bool write_mode,bool write_only=false);
		~Session();
	private:
		std::fstream* m_stream;
		std::string m_file_path;

		#ifdef ND_DEBUG
		bool m_write_only;
		bool m_write_mode;
		#endif
	public:
		World* genWorldFile(const WorldIOInfo& info);

		void saveWorld(World * world);

		World* loadWorld();

		void loadChunk(Chunk* chunk, int offset);

		void saveChunk(Chunk* chunk, int offset);

		void close();

	};
}


