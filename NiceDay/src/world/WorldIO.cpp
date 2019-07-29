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


namespace WorldIO
{
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


	World* Session::genWorldFile(const WorldIOInfo& info)
	{
		World* ww = new World(m_file_path, info.world_name.c_str(), info.chunk_width, info.chunk_height);
		World& w = *ww;
		w.m_info.terrain_level = info.terrain_level;
	

		//creating world file
		m_stream->write((const char*)&w.getInfo(), sizeof(WorldInfo));
		auto buff = new char[HEADER_SIZE - sizeof(WorldInfo)];
		memset(buff, 0, HEADER_SIZE - sizeof(WorldInfo));
		m_stream->write(buff, HEADER_SIZE - sizeof(WorldInfo)); //add some space to fill the HEADER_SIZE
		delete[] buff;

		Chunk* cc = new Chunk();
		memset(cc, 0, sizeof(Chunk));
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
		if (m_stream->rdstate() & std::fstream::failbit) //cannot read file
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

	void Session::loadChunk(Chunk* chunk, int offset)
	{
		m_stream->seekg(HEADER_SIZE + sizeof(Chunk) * offset, std::ios::beg);
		m_stream->read((char*)chunk, sizeof(Chunk));
		chunk->m_main_thread_fence = 0;
		chunk->m_light_thread_fence = 0;
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::saveChunk(Chunk* chunk, int offset)
	{
		m_stream->seekp(HEADER_SIZE + sizeof(Chunk) * offset, std::ios::beg);
		m_stream->write((char*)chunk, sizeof(Chunk));
		CHECK_STREAM_STATE(m_stream);
	}

	void Session::close()
	{
		m_stream->close();
	}



	struct BigHeaderHeader
	{
		uint32_t free_segment_count;
		uint32_t segment_count;
		uint32_t chunk_id_count;
	};

	uint32_t DynamicSaver::getChunkOffset(int chunkID)
	{
		auto it = m_chunk_offsets.find(chunkID);
		if (it != m_chunk_offsets.end())
			return m_chunk_offsets[chunkID];

		if (!m_free_offsets.empty())
		{
			auto f = m_free_offsets.front();
			m_free_offsets.pop_front();
			m_chunk_offsets[chunkID] = f;
			return f;
		}
		return m_segment_count++;
	}

	void DynamicSaver::eraseSegment(uint32_t offset)
	{
		ChunkSegmentHeader head;
		m_stream->seekg(m_TOTAL_OFFSET + offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
		m_stream->read((char*)&head, sizeof(ChunkSegmentHeader));

		std::vector<uint32_t> offsetsToDestroy;

		offsetsToDestroy.push_back(offset);
		while (head.hasNext())
		{
			offsetsToDestroy.push_back(head.next_index);
			m_stream->seekg(m_TOTAL_OFFSET + head.next_index * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
			m_stream->read((char*)&head, sizeof(ChunkSegmentHeader));
		}

		head = ChunkSegmentHeader();
		for (auto offset_to_destroy : offsetsToDestroy)
		{
			m_free_offsets.push_back(offset_to_destroy);
			m_stream->seekp(m_TOTAL_OFFSET + offset_to_destroy * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
			m_stream->write((char*)&head, sizeof(ChunkSegmentHeader));
		}
	}

	void DynamicSaver::loadVTable()
	{
		if (!std::filesystem::exists(m_path)) {
			//we have to check if file even exists, if not we gotta make it
			m_stream = new std::fstream(m_path, std::ios::out | std::ios::binary);
			CHECK_STREAM_STATE_START(m_stream, m_path);
			/*m_stream->seekp(std::ios::end);
			if (m_stream->tellp() == 0)//no file here
			{
				char c = 0;
				m_stream->write(&c, 1);
			}*/
			m_stream->close();
			delete m_stream;
		}

		beginSession();



		m_stream->seekg(0,std::ios::end);
		long long length = m_stream->tellg();

		if (length < m_BASE_TOTAL_OFFSET + sizeof(BigHeaderHeader)) //never ever was it created
		{
			BigHeaderHeader header = {};
			m_stream->seekp(m_BASE_TOTAL_OFFSET);
			m_stream->write((char*)&header, sizeof(BigHeaderHeader));
			m_stream->seekp(m_TOTAL_OFFSET); //reserve whole header section
			m_segment_count = 0;
		}
		else
		{
			BigHeaderHeader header = {};
			m_stream->seekg(m_BASE_TOTAL_OFFSET);
			m_stream->read((char*)&header, sizeof(BigHeaderHeader));
			m_segment_count = header.segment_count;

			//m_stream->seekg(m_BASE_TOTAL_OFFSET+sizeof(BigHeaderHeader));
			for (int i = 0; i < header.free_segment_count; ++i)
			{
				uint32_t o = 0;
				m_stream->read((char*)&o, sizeof(uint32_t));
				m_free_offsets.push_back(o);
			}
			m_stream->seekg(
				m_BASE_TOTAL_OFFSET + sizeof(BigHeaderHeader) + DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint32_t));
			for (int i = 0; i < header.chunk_id_count; ++i)
			{
				pai p;
				m_stream->read((char*)&p, sizeof(pai));
				m_chunk_offsets[p.id] = p.offset;
			}
		}
		endSession();
	}

	DynamicSaver::~DynamicSaver()
	{
		endSession();
		beginSession();
		saveVTable();
		endSession();
	}

	void DynamicSaver::init()
	{
		loadVTable();
	}

	void DynamicSaver::saveVTable()
	{
		BigHeaderHeader header;

		header.segment_count = m_segment_count;
		header.free_segment_count = m_free_offsets.size();
		header.chunk_id_count = m_chunk_offsets.size();

		m_stream->seekp(m_BASE_TOTAL_OFFSET);
		m_stream->write((char*)&header, sizeof(BigHeaderHeader));

		//m_stream->seekg(m_BASE_TOTAL_OFFSET+sizeof(BigHeaderHeader));
		for (uint32_t free_offset : m_free_offsets)
			m_stream->write((char*)&free_offset, sizeof(uint32_t));

		m_stream->seekp(m_BASE_TOTAL_OFFSET + sizeof(BigHeaderHeader) + DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint32_t));
		for (auto pair : m_chunk_offsets)
		{
			pai p{pair.first,pair.second};
			m_stream->write((char*)&p, sizeof(pai));
		}
	}

	DynamicSaver::DynamicSaver(std::string path, uint32_t totalOffset)
		: m_BASE_TOTAL_OFFSET(totalOffset), m_path(path)
	{
		m_TOTAL_OFFSET =
			m_BASE_TOTAL_OFFSET +
			sizeof(BigHeaderHeader) +
			DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint32_t) +
			DYNAMIC_SAVER_CHUNK_ID_COUNT * (sizeof(int) + sizeof(uint32_t));
	}

	void DynamicSaver::beginSession()
	{
		m_stream = new std::fstream(m_path, std::ios::in | std::ios::out | std::ios::binary);
		CHECK_STREAM_STATE_START(m_stream, m_path);
	}

	void DynamicSaver::endSession()
	{
		if (m_stream) {
			m_stream->close();
			delete m_stream;
			m_stream = nullptr;
		}
	}

	void DynamicSaver::setWriteChunkID(int chunkID)
	{
		
		m_writeChunkID = chunkID;
		auto oldSegmentCount = m_segment_count;
		m_p_offset = getChunkOffset(chunkID);
		if (oldSegmentCount != m_segment_count) //we have added new segment
		{
			m_stream->seekp(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
			m_write_header = ChunkSegmentHeader();
			m_stream->write((char*)&m_write_header, sizeof(ChunkSegmentHeader));
		}
		else
		{
			m_stream->seekg(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
			m_stream->read((char*)&m_write_header, sizeof(ChunkSegmentHeader));
			m_stream->seekp(
				m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE + sizeof(ChunkSegmentHeader));
		}
		m_chunk_offsets[chunkID] = m_p_offset;
		m_p_byte_reciproc = DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader);
	}

	void DynamicSaver::flushWrite()
	{
		if (m_write_header.hasNext())
		{
			//kill all bastard-ish children
			eraseSegment(m_write_header.next_index);
		}
		if (m_p_byte_reciproc == DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader))
			//nothing was written to that chunk
		{
			m_free_offsets.push_back(m_p_offset);
			m_chunk_offsets.erase(m_chunk_offsets.find(m_writeChunkID)); //kill reference from id to segment
		}

		//remove memories (no next Segment)
		m_stream->seekp(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
		m_write_header = ChunkSegmentHeader();
		m_stream->write((char*)&m_write_header, sizeof(ChunkSegmentHeader));
	}

	bool DynamicSaver::setReadChunkID(int chunkID)
	{
		if (m_chunk_offsets.find(chunkID) == m_chunk_offsets.end()) //this chunk does not exist
			return false;
		m_g_offset = m_chunk_offsets[chunkID];
		m_stream->seekg(m_TOTAL_OFFSET + m_g_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
		m_stream->read((char*)&m_read_header, sizeof(ChunkSegmentHeader));
		m_g_byte_reciproc = DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader);

		return true;
	}

	void DynamicSaver::write(const char* b, uint32_t length)
	{
		if (m_p_byte_reciproc < length)
		{
			if (m_p_byte_reciproc != 0)
				m_stream->write(b, m_p_byte_reciproc);
			auto b_start_offset = m_p_byte_reciproc;

			m_p_byte_reciproc = DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader);

			if (m_write_header.hasNext())
			{
				m_p_offset = m_write_header.next_index;
				m_stream->seekg(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
				m_stream->read((char*)&m_write_header, sizeof(ChunkSegmentHeader));
				m_stream->seekp(
					m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE + sizeof(ChunkSegmentHeader));
			}
			else
			{
				uint32_t nextOffset;
				if (!m_free_offsets.empty())
				{
					nextOffset = m_free_offsets.front();
					m_free_offsets.pop_front();
				}
				else
					nextOffset = m_segment_count++;

				//set pointer to next segment in old segment
				m_write_header.next_index = nextOffset;
				m_stream->seekp(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
				m_stream->write((char*)&m_write_header, sizeof(ChunkSegmentHeader));

				//get the new segment and set pointer to next segment to invalid
				m_p_offset = nextOffset;
				m_write_header = ChunkSegmentHeader();
				m_stream->seekp(m_TOTAL_OFFSET + m_p_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
				m_stream->write((char*)&m_write_header, sizeof(ChunkSegmentHeader));
			}
			write(b + b_start_offset, length - b_start_offset);
			//m_stream->write(b + b_start_offset, length - b_start_offset);
			//m_p_byte_reciproc -= length - b_start_offset;
		}
		else
		{
			m_stream->write(b, length);
			m_p_byte_reciproc -= length;
		}
	}

	bool DynamicSaver::read(char* b, uint32_t length)
	{
		if (m_g_byte_reciproc < length)
		{
			if (m_g_byte_reciproc != 0)
				m_stream->read(b, m_g_byte_reciproc);
			auto b_start_offset = m_g_byte_reciproc;

			m_g_byte_reciproc = DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader);

			if (m_read_header.hasNext())
			{
				m_g_offset = m_read_header.next_index;
				m_stream->seekg(m_TOTAL_OFFSET + m_g_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
				m_stream->read((char*)&m_read_header, sizeof(ChunkSegmentHeader));
			}
			else return false;

			read(b + b_start_offset, length - b_start_offset);
		}
		else
		{
			m_stream->read(b, length);
			m_g_byte_reciproc -= length;
		}
		return true;
	}
}
