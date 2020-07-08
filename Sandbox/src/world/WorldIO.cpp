#include "ndpch.h"
#include "WorldIO.h"
#include "World.h"
#include <fstream>
#include <utility>
#include <filesystem>


#ifdef ND_DEBUG
#define CHECK_STREAM_STATE(x) checkDebugState(x)
#define CHECK_STREAM_STATE_START(x,y) if(checkDebugState(x)){ND_ERROR("WorldIO: Check if filepath is valid: \"{}\"",y);assert(false);}
#else
#define CHECK_STREAM_STATE(x)
#define CHECK_STREAM_STATE_START(x,y)

#endif

static bool checkDebugState(std::fstream* stream)
{
	auto t = stream->rdstate();
	auto fv = false;
	if ((t & std::fstream::goodbit) != 0)
		ND_ERROR("WorldIO: goodbit");
	if ((t & std::fstream::eofbit) != 0)
		ND_ERROR("WorldIO: eofbit");
	if ((t & std::fstream::badbit) != 0)
		ND_ERROR("WorldIO: badbit");
	if ((t & std::fstream::failbit) != 0)
	{
		ND_ERROR("WorldIO: failbit");
		fv = true;
	}
	//if (fv)
	//	ASSERT(false, "");
	return fv;
}

//in bytes
static const int HEADER_SIZE = 1000;
static const int HEADER_LOADED_CHUNKS_SIZE = 1000000;//1MB reserved for list of chunks which have been already generated

static const int HEADER_START_POINT = 0;
static const int HEADER_TOTAL_START_POINT = HEADER_SIZE;
static const int HEADER_TOTAL_END_POINT = HEADER_SIZE+ HEADER_LOADED_CHUNKS_SIZE;


namespace WorldIO
{
	//=========================StaticSaver========================

	Session::Session(const std::string& path, bool write_mode, bool write_only)
	:m_file_path(path)
	{
		m_stream = new std::fstream(
			path, (write_only ? 0 : std::ios::in) | (write_mode ? std::ios::out : 0) | std::ios::binary);
		CHECK_STREAM_STATE_START(m_stream, path);
	}

	Session::~Session()
	{
		delete m_stream;
	}


	void Session::genWorldFile(const WorldInfo* info)
	{
		saveWorldMetadata(info);
		m_stream->seekp(HEADER_TOTAL_START_POINT);
		
		char* b = new char[HEADER_LOADED_CHUNKS_SIZE];
		memset(b, 0, HEADER_LOADED_CHUNKS_SIZE);
		m_stream->write(b, HEADER_LOADED_CHUNKS_SIZE);
		delete[] b;
		
		m_stream->seekp(HEADER_TOTAL_END_POINT);

		auto cc = new Chunk();
		memset(cc, 0, sizeof(Chunk));
		cc->last_save_time = 0;
		//writing blank chunks
		for (int y = 0; y < info->chunk_height; y++)
		{
			for (int x = 0; x < info->chunk_width; x++)
			{
				cc->m_x = x;
				cc->m_y = y;

				m_stream->write((char*)cc, sizeof(Chunk));
			}
		}
		delete cc;
	}

	bool Session::loadWorldMetadata(WorldInfo* world)
	{
		ND_TRACE("Loading world {0}", m_file_path);
		m_stream->seekg(0,std::ios::end);
		if (m_stream->tellg() < sizeof(WorldInfo))//this file doesn't seem to exist
			return false;
		m_stream->seekg(HEADER_START_POINT);
		m_stream->read((char*)world, sizeof(WorldInfo));
		CHECK_STREAM_STATE(m_stream);
		return true;
	}

	void Session::saveWorldMetadata(const WorldInfo* world)
	{
		m_stream->seekp(HEADER_START_POINT);
		m_stream->write((char*)world, sizeof(WorldInfo));
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::saveGenBoolMap(const NDUtil::Bitset* bitset)
	{
		m_stream->seekp(HEADER_TOTAL_START_POINT);
		bitset->write(*m_stream);
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::loadGenBoolMap(NDUtil::Bitset* bitset)
	{
		m_stream->seekg(HEADER_TOTAL_START_POINT);
		bitset->read(*m_stream);
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::loadChunk(Chunk* chunk, int offset)
	{
		m_stream->seekg(HEADER_TOTAL_END_POINT + sizeof(Chunk) * offset, std::ios::beg);
		m_stream->read((char*)chunk, sizeof(Chunk));
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::saveChunk(const Chunk* chunk, int offset)
	{
		m_stream->seekp(HEADER_TOTAL_END_POINT + sizeof(Chunk) * offset, std::ios::beg);
		m_stream->write((char*)chunk, sizeof(Chunk));
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::close()
	{
		m_stream->close();
	}

	IBinaryStream::RWStream streamFuncs(DynamicSaver* saver)
	{
		return IBinaryStream::RWStream(
			std::bind(&DynamicSaver::readI, saver, std::placeholders::_1, std::placeholders::_2),
			std::bind(&DynamicSaver::writeI, saver, std::placeholders::_1, std::placeholders::_2)
		);
	}
}
