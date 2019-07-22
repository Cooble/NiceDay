#include "ndpch.h"
#include "WorldIO.h"
#include "World.h"
#include <fstream>



#ifdef ND_DEBUG
#define CHECK_STREAM_STATE(x) checkDebugState(x)
#define CHECK_STREAM_STATE_START(x,y) if(checkDebugState(x)){ND_ERROR("WorldIO: Check if filepath is valid: \"{}\"",y);}
#else
#define CHECK_STREAM_STATE(x)
#define CHECK_STREAM_STATE_START(x,y)

#endif

static bool checkDebugState(std::fstream* stream) {
	auto t = stream->rdstate();
	auto fv = false;
	if ((t & std::fstream::goodbit) != 0)
		ND_ERROR("WorldIO: goodbit");
	if ((t & std::fstream::eofbit) != 0)
		ND_ERROR("WorldIO: eofbit");
	if ((t & std::fstream::badbit) != 0)
		ND_ERROR("WorldIO: badbit");
	if ((t & std::fstream::failbit) != 0) {
		ND_ERROR("WorldIO: failbit");
		fv = true;
	}
	//if (fv)
	//	ASSERT(false, "");
	return fv;
}

//in bytes
static const int HEADER_SIZE = 1000;


namespace WorldIO {
	Session::Session(const std::string& path, bool write_mode,bool write_only):
		m_file_path(path)
	{
		m_stream = new std::fstream(path, (write_only?0:std::ios::in) | (write_mode ? std::ios::out : 0) | std::ios::binary);
		CHECK_STREAM_STATE_START(m_stream,path);
	}

	Session::~Session()
	{
		delete m_stream;
	}


	World* Session::genWorldFile(const WorldIOInfo & info)
	{
		World* ww = new World(m_file_path, info.world_name.c_str(), info.chunk_width, info.chunk_height);
		World& w = *ww;
		w.m_info.terrain_level = info.terrain_level;

		//creating world file
		m_stream->write((const char*)&w.getInfo(), sizeof(WorldInfo));
		auto buff = new char[HEADER_SIZE - sizeof(WorldInfo)];
		memset(buff, 0, HEADER_SIZE - sizeof(WorldInfo));
		m_stream->write(buff, HEADER_SIZE - sizeof(WorldInfo));//add some space to fill the HEADER_SIZE
		delete[] buff;

		Chunk* cc = new Chunk();
		memset(cc, 0,  sizeof(Chunk));
		cc->setLoaded(false);
		cc->last_save_time = 0;
		//writing blank chunks
		for (int y = 0; y < info.chunk_height; y++)
		{
			for (int x = 0; x < info.chunk_width; x++)
			{
				
				cc->m_x = x;
				cc->m_y = y;

				m_stream->write((char*)cc, sizeof(Chunk));
			}

		}
		delete cc;
		return ww;
	}
	
	World* Session::loadWorld()
	{
		ND_TRACE("Loading world {0}", m_file_path);
		WorldInfo info;
		m_stream->read((char*)&info, sizeof(WorldInfo));
		CHECK_STREAM_STATE(m_stream);
		if (m_stream->rdstate() & std::fstream::failbit)//cannot read file
			return nullptr;
		World* ww = new World(m_file_path, &info);
		return ww;
	}

	//saves only worldinfo
	void Session::saveWorld(World* world)
	{

		m_stream->write((char*)&world->getInfo(), sizeof(WorldInfo));
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::loadChunk(Chunk* chunk, int offset) {
		m_stream->seekg(HEADER_SIZE + sizeof(Chunk)*offset, std::ios::beg);
		m_stream->read((char*)chunk, sizeof(Chunk));
		chunk->m_main_thread_fence = 0;
		chunk->m_light_thread_fence = 0;
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::saveChunk(Chunk* chunk, int offset) {
		m_stream->seekp(HEADER_SIZE + sizeof(Chunk)*offset, std::ios::beg);
		m_stream->write((char*)chunk, sizeof(Chunk));
		CHECK_STREAM_STATE(m_stream);
	}
	
	void Session::close() {
		m_stream->close();
	}
}

