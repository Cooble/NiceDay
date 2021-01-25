#pragma once


namespace NDUtil
{
	/***
 * Lock-free buffer
 * one    thread can call write() and push()
 * second thread can call read() and  pop()
 *
 * if buffer is full you cannot write more
 * if buffer is empty you cannot read more
 *
 * Buffer can be constructed without external mem allocation using template args
 * Buffer can be constructed without template args but need to call allocate to obtain memory
 */
	template <size_t Size = 0, size_t FrameCount = 0, bool MultiThread = true>
	class RingBuffer
	{
		static_assert(Size == 0 ? FrameCount == 0 : true, "Size cannot be zero if FrameCount is not");
		static_assert(Size == 0 || (Size % FrameCount) == 0, "Size has to be divisible by FrameCount");
	private:
		using IndexType = typename std::conditional<MultiThread, std::atomic<size_t>, size_t>::type;
		
		const size_t m_template_size = Size;
		size_t m_frame_size = 0;
		size_t m_frame_count = FrameCount;

		IndexType m_producer_index = 0;
		IndexType m_consumer_index = 0;

		IndexType m_reads = 0;
		IndexType m_writes = 0;

		char* m_buffer = nullptr;
		char m_buf[Size];

	public:
		RingBuffer()
		{
			if (Size != 0) {
				m_buffer = m_buf;
				m_frame_size = Size / m_frame_count;
			}
		}
		RingBuffer(size_t frameSize, size_t frameCount)
		{
			allocate(frameSize, frameCount);
		}


		void allocate(size_t frameSize, size_t frameCount)
		{
			ASSERT(m_frame_count == 0, "this buffer has been already allocated!");
			m_frame_size = frameSize;
			m_frame_count = frameCount;
			m_producer_index = 0;
			m_consumer_index = 0;
			m_reads = 0;
			m_writes = 0;
			ASSERT(frameSize * frameCount, "Cannot be zero");
			m_buffer = (char*)malloc(getBufferSize());
			ZeroMemory(m_buffer, getBufferSize());

		}
		~RingBuffer()
		{
			if (m_template_size == 0 && m_buffer)
				free(m_buffer);
		}

		/**
		 * Returns available frame where read head points to
		 * or nullptr if none is available
		 * @NOTE: to move to other frame call pop()
		 */
		const char* read()
		{
			//no data
			if (!getAvailableReads())
				return nullptr;
			return m_buffer + m_consumer_index * m_frame_size;
		}
		/**
		 * Returns available frame where write head points to
		 * or nullptr if none is available
		 * @NOTE: to move to other frame call push()
		 */
		char* write()
		{
			if (!getAvailableWrites())
				return nullptr;
			return m_buffer + m_producer_index * m_frame_size;
		}

		/**
		 * Moves head onto the next frame
		 * return true if success
		 */
		bool pop()
		{
			if (!getAvailableReads())
				return false;

			m_consumer_index = (m_consumer_index + 1) % m_frame_count;
			++m_reads;
			return true;
		}
		/**
		 * Moves head onto the next frame
		 * return true if success
		 */
		bool push()
		{
			if (!getAvailableWrites())
				return false;

			m_producer_index = (m_producer_index + 1) % m_frame_count;
			++m_writes;
			return true;
		}

		inline size_t getBufferSize() const { return m_frame_count * m_frame_size; }
		inline size_t getFrameSize() const { return m_frame_size; }
		inline size_t getFrameCount() const { return m_frame_count; }

		/**
		 * Number of frames that can be read
		 */
		size_t getAvailableReads() const
		{
			return std::max((unsigned long long)0, (unsigned long long)(m_writes - m_reads));

		}
		size_t getAvailableWrites() const
		{
			return std::max((unsigned long long)0, (unsigned long long)(m_frame_count - getAvailableReads()));

		}
		inline bool isEmpty() const { return m_reads == m_writes; }
		inline bool isFull() const { return getAvailableWrites() == 0; }
		// dont call when it's being used
		inline void clear()
		{
			m_reads = 0;
			m_writes = 0;
			m_consumer_index = 0;
			m_producer_index = 0;
		}
	};

	/**
	 * Type of ringbuffer that needs to call allocate
	 */
	typedef RingBuffer<0, 0> RingBufferLite;
}
