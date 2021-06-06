#pragma once
#include <functional>

namespace nd {
namespace IBinaryStream {

	typedef std::function<void(const char* c, size_t length)> WriteFunc;
	typedef std::function<void(char* c, size_t length)> ReadFunc;

	struct RWStream
	{
		WriteFunc m_write;
		ReadFunc m_read;

		RWStream(ReadFunc read, WriteFunc write) : m_write(std::move(write)), m_read(std::move(read))
		{
		}

		template <typename T>
		void write(T& t) const
		{
			m_write((char*)&t, sizeof(T));
		}

		template <typename T>
		void read(T& t) const
		{
			m_read((char*)&t, sizeof(T));
		}

		inline void write(const char* c, size_t size) const { m_write(c, size); }
		inline void read(char* c, size_t size) const { m_read(c, size); }
	};

	typedef std::function<void(const ReadFunc&)> ReaderFunc;
	typedef std::function<void(const WriteFunc&)> WriterFunc;
	typedef std::function<void(const RWStream&)> RWFunc;

}

#define ND_IBS_HOOK(function,pointer) std::bind(function,pointer,std::placeholders::_1)

#define ND_IBS_FUNC(function,pointer) std::bind(function,pointer,std::placeholders::_1,std::placeholders::_2)
#define ND_IBS_WRITE(function,pointer) std::make_pair(ReadFunc(),ND_IBS_FUNC(function,pointer))
#define ND_IBS_READ(function,pointer) std::make_pair(ND_IBS_FUNC(function,pointer),WriteFunc())
}
