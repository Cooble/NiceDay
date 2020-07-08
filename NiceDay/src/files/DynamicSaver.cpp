#include "ndpch.h"
#include "DynamicSaver.h"



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

struct BigHeaderHeader
{
	uint64_t free_segment_count;
	uint64_t segment_count;
	uint64_t chunk_id_count;
};

uint64_t DynamicSaver::getChunkOffset(_DSChunkID chunkID)
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

void DynamicSaver::eraseSegment(uint64_t offset)
{
	ChunkSegmentHeader head;
	m_stream->seekg(m_TOTAL_OFFSET + offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
	m_stream->read((char*)&head, sizeof(ChunkSegmentHeader));

	std::vector<uint64_t> offsetsToDestroy;

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



	m_stream->seekg(0, std::ios::end);
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
			uint64_t o = 0;
			m_stream->read((char*)&o, sizeof(uint64_t));
			m_free_offsets.push_back(o);
		}
		m_stream->seekg(
			m_BASE_TOTAL_OFFSET + sizeof(BigHeaderHeader) + DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint64_t));
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
	if (isOpened())
		endSession();
}

void DynamicSaver::init()
{
	loadVTable();
}
bool DynamicSaver::initIfExist()
{
	if (!std::filesystem::exists(m_path))
		return false;
	loadVTable();
	return true;
}

void DynamicSaver::saveVTable()
{
	m_dirty_vtable = false;
	BigHeaderHeader header;

	header.segment_count = m_segment_count;
	header.free_segment_count = m_free_offsets.size();
	header.chunk_id_count = m_chunk_offsets.size();

	m_stream->seekp(m_BASE_TOTAL_OFFSET);
	m_stream->write((char*)&header, sizeof(BigHeaderHeader));

	//m_stream->seekg(m_BASE_TOTAL_OFFSET+sizeof(BigHeaderHeader));
	for (auto free_offset : m_free_offsets)
		m_stream->write((char*)&free_offset, sizeof(uint64_t));

	m_stream->seekp(m_BASE_TOTAL_OFFSET + sizeof(BigHeaderHeader) + DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint64_t));
	for (auto pair : m_chunk_offsets)
	{
		pai p{ pair.first,pair.second };
		m_stream->write((char*)&p, sizeof(pai));
	}
}

DynamicSaver::DynamicSaver(std::string path, uint64_t totalOffset)
	: m_BASE_TOTAL_OFFSET(totalOffset), m_path(path)
{
	m_TOTAL_OFFSET =
		m_BASE_TOTAL_OFFSET +
		sizeof(BigHeaderHeader) +
		DYNAMIC_SAVER_FREE_SEGMENTS * sizeof(uint64_t) +
		DYNAMIC_SAVER_CHUNK_ID_COUNT * (sizeof(int) + sizeof(uint64_t));
}

void DynamicSaver::beginSession()
{
	if (!m_is_opened) {
		m_stream = new std::fstream(m_path, std::ios::in | std::ios::out | std::ios::binary);
		CHECK_STREAM_STATE_START(m_stream, m_path);
		m_is_opened = true;
	}
}

void DynamicSaver::endSession()
{
	if (m_stream) {
		if (!m_has_flushed)
			flushWrite();
		if (m_dirty_vtable)//always save vtable just to be sure
			saveVTable();
		m_stream->close();
		delete m_stream;
		m_stream = nullptr;
	}
	m_is_opened = false;

}

void DynamicSaver::setWriteChunkID(_DSChunkID chunkID)
{

	if (!m_has_flushed)
		flushWrite();
	m_has_flushed = false;
	m_dirty_vtable = true;
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
	m_has_flushed = true;
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
	m_stream->flush();
}

bool DynamicSaver::setReadChunkID(_DSChunkID chunkID)
{
	if (m_chunk_offsets.find(chunkID) == m_chunk_offsets.end()) //this chunk does not exist
		return false;
	m_g_offset = m_chunk_offsets[chunkID];
	m_stream->seekg(m_TOTAL_OFFSET + m_g_offset * DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE);
	m_stream->read((char*)&m_read_header, sizeof(ChunkSegmentHeader));
	m_g_byte_reciproc = DYNAMIC_SAVER_CHUNK_SEGMENT_BYTE_SIZE - sizeof(ChunkSegmentHeader);

	return true;
}

void DynamicSaver::write(const char* b, uint64_t length)
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
			uint64_t nextOffset;
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

bool DynamicSaver::read(char* b, uint64_t length)
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

void DynamicSaver::clearEverything()
{
	std::filesystem::remove(m_path);
}