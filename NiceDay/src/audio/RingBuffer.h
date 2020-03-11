#pragma once


/***
 * Lock-free buffer
 * one    thread can call write() and push()
 * second thread can call read() and  pop()
 *
 * if buffer is full you cannot write more
 * if buffer is empty you cannot read more
 */
class RingBuffer
{
private:
	const size_t m_frame_size;
	const size_t m_frame_count;
	char* m_buffer;
	std::atomic<size_t> m_producer_index;
	std::atomic<size_t> m_consumer_index;
	bool m_read_priority;

	std::atomic<size_t> m_reads;
	std::atomic<size_t> m_writes;
	
public:
	RingBuffer(size_t frameSize, size_t frameCount);
	~RingBuffer();

	/**
	 * Returns available frame where read head points to
	 * or nullptr if none is available
	 * @NOTE: to move to other frame call pop()
	 */
	const char* read();
	/**
	 * Returns available frame where write head points to
	 * or nullptr if none is available
	 * @NOTE: to move to other frame call push()
	 */
	char* write();

	/**
	 * Moves head onto the next frame
	 * return true if success
	 */
	bool pop();
	/**
	 * Moves head onto the next frame
	 * return true if success
	 */
	bool push();

	inline size_t getBufferSize() const { return m_frame_count * m_frame_size; }
	inline size_t getFrameSize() const { return m_frame_size; }
	inline size_t getFrameCount() const { return m_frame_count; }

	/**
	 * Number of frames that can be read
	 */
	size_t getAvailableReads() const;

	
	
};
