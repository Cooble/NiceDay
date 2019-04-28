#pragma once
#include "ndpch.h"

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
	public:
		World* genWorld(const WorldIOInfo& info);

		void saveWorld(World * world);

		World* loadWorld();

		void loadChunk(Chunk* chunk, int offset);

		void saveChunk(Chunk* chunk, int offset);

		void close();

	};
}


