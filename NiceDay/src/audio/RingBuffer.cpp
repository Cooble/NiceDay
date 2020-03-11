#include "ndpch.h"
#include "RingBuffer.h"

RingBuffer::RingBuffer(size_t frameSize, size_t frameCount)
	:m_frame_size(frameSize),
	m_frame_count(frameCount),
	m_producer_index(0),
	m_consumer_index(0),
	m_read_priority(false),
	m_reads(0),
	m_writes(0)

{
	ASSERT(frameSize * frameCount, "Cannot be zero");
	m_buffer = (char*) malloc(getBufferSize());
	ZeroMemory(m_buffer, getBufferSize());
}

RingBuffer::~RingBuffer()
{
	free(m_buffer);
}

const char* RingBuffer::read()
{
	//no data
	if(!m_read_priority && m_consumer_index==m_producer_index)
		return nullptr;
	return m_buffer + m_consumer_index * m_frame_size;
}

char* RingBuffer::write()
{
	if (m_read_priority && m_producer_index == m_consumer_index)
		return nullptr;
	
	return m_buffer + m_producer_index * m_frame_size;
}

bool RingBuffer::pop()
{
	if (!m_read_priority && m_consumer_index == m_producer_index)
		return false;
	m_consumer_index = (m_consumer_index + 1) % m_frame_count;
	++m_reads;
	return true;
}

bool RingBuffer::push()
{
	
	if (m_read_priority && m_producer_index == m_consumer_index)
		return false;
	
	m_producer_index = (m_producer_index + 1) % m_frame_count;
	++m_writes;
	m_read_priority = m_producer_index == m_consumer_index;

	return true;
}

size_t RingBuffer::getAvailableReads() const
{
	return m_writes - m_reads;
}


