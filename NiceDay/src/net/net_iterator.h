#pragma once
#include "ndpch.h"
#include "net_iterator.h"
#include "net/net.h"

namespace nd::net
{
struct NetReader;
struct NetWriter;


struct Serializable
{
	//using CheckFunExist = std::void_t<decltype(&Self::deserialize), decltype(&Self::serialize)>;

	/*bool deserialize(NetReader& reader)
	{
		return false;
	}

	void serialize(NetWriter& reader) const
	{
	}*/
};

namespace NetConversions
{
	template <typename T>
	void serialize(const T& from, NetWriter& r) = delete;
	template <typename T>
	bool deserialize(T& to, NetReader& r) = delete;

	template <typename T>
	bool deserializeArray(std::vector<T>& to, NetReader& r);
	template <typename T>
	void serializeArray(const std::vector<T>& from, NetWriter& r);


	template <typename T, class = void>
	struct is_convertible : std::false_type
	{
	};

	template <typename T>
	struct is_convertible<T, decltype(serialize<T>(std::declval<const T&>(), std::declval<NetWriter&>()), void()
	                      )> : std::true_type
	{
	};

	template <class T>
	struct is_array
	{
		static constexpr bool value = false;
	};

	template <class T>
	struct is_array<std::vector<T>>
	{
		static constexpr bool value = true;
	};
}

struct NetReader : nd::net::BufferReader
{
	NetReader(nd::net::Buffer& b) : BufferReader(b)
	{
	}

	template <typename T>
	bool get(T& t)
	{
		if constexpr (std::is_base_of_v<Serializable, T>)
			return t.deserialize(*this);
		if constexpr (NetConversions::is_convertible<T>::value)
		{
			auto ret = NetConversions::deserialize(t, *this);
			return ret;
		}
		if constexpr (NetConversions::is_array<T>::value)
		{
			return NetConversions::deserializeArray(t, *this);
		}

		auto out = BufferReader::tryRead<T>();
		if (!out)
			return false;
		t = *out;
		return true;
	}

	bool get(std::string& s, int maxLength)
	{
		s = readStringUntilNull(maxLength);
		return !isStringError();
	}
};

struct NetWriter : nd::net::BufferWriter
{
	NetWriter(nd::net::Buffer& b, bool append = false, bool expand = true) : BufferWriter(b, append, expand)
	{
	}

	template <typename T>
	void put(const T& t)
	{
		if constexpr (std::is_base_of_v<Serializable, T>)
		{
			t.serialize(*this);
			return;
		}
		if constexpr (NetConversions::is_convertible<T>::value)
		{
			NetConversions::serialize(t, *this);
			return;
		}
		if constexpr (NetConversions::is_array<T>::value)
		{
			NetConversions::serializeArray(t, *this);
			return;
		}

		BufferWriter::write<T>(t);
	}
};

inline NetWriter NetAppender(Buffer& b) { return {b, true, true}; }

namespace NetConversions
{
	template <>
	inline void serialize<uint32_t>(const uint32_t& from, NetWriter& r)
	{
		r.write(std::to_string(from));
	}

	template <>
	inline void serialize<uint64_t>(const uint64_t& from, NetWriter& r)
	{
		r.write(std::to_string(from));
	}

	template <>
	inline void serialize<int>(const int& from, NetWriter& r)
	{
		r.write(std::to_string(from));
	}

	template <>
	inline void serialize<float>(const float& from, NetWriter& r)
	{
		r.write(std::to_string(from));
	}

	template <>
	inline void serialize<double>(const double& from, NetWriter& r)
	{
		r.write(std::to_string(from));
	}
	template <>
	inline void serialize<bool>(const bool& from, NetWriter& r)
	{
		int i = from;
		serialize<int>(i, r);
	}

	template <>
	inline void serialize<std::string>(const std::string& from, NetWriter& r)
	{
		r.write(from);
	}

	template <>
	inline void serialize<glm::vec2>(const glm::vec2& from, NetWriter& r)
	{
		r.put(from.x);
		r.put(from.y);
	}

	template <>
	inline void serialize<glm::ivec2>(const glm::ivec2& from, NetWriter& r)
	{
		r.put(from.x);
		r.put(from.y);
	}

	template <typename T>
	void serializeArray(const std::vector<T>& from, NetWriter& r)
	{
		r.put(from.size());

		for (auto& t : from)
			r.put(t);
	}

	// ===============================================

	template <>
	inline bool deserialize<uint32_t>(uint32_t& to, NetReader& r)
	{
		auto s = r.readStringUntilNull(11);
		auto c = const_cast<char*>(s.c_str());
		errno = 0;
		auto out = strtoul(s.c_str(), &c, 10);
		if (errno)
			return false;

		to = out;
		return true;
	}

	template <>
	inline bool deserialize<uint64_t>(uint64_t& to, NetReader& r)
	{
		auto s = r.readStringUntilNull(11);
		auto c = const_cast<char*>(s.c_str());
		errno = 0;
		auto out = strtoull(s.c_str(), &c, 10);
		if (errno)
			return false;

		to = out;
		return true;
	}

	template <>
	inline bool deserialize<int>(int& to, NetReader& r)
	{
		auto s = r.readStringUntilNull(11);
		auto c = const_cast<char*>(s.c_str());
		errno = 0;
		auto out = strtol(s.c_str(), &c, 10);
		if (errno)
			return false;

		to = out;
		return true;
	}
	template <>
	inline bool deserialize<bool>(bool& to, NetReader& r)
	{
		int i;
		if (!deserialize(i, r))
			return false;
		to = i;
		return true;
	}

	template <>
	inline bool deserialize<float>(float& to, NetReader& r)
	{
		auto s = r.readStringUntilNull(-1);
		auto c = const_cast<char*>(s.c_str());
		errno = 0;
		auto out = strtof(s.c_str(), &c);
		if (errno)
			return false;

		to = out;
		return true;
	}

	template <>
	inline bool deserialize<double>(double& to, NetReader& r)
	{
		auto s = r.readStringUntilNull(-1);
		auto c = const_cast<char*>(s.c_str());
		errno = 0;
		auto out = strtod(s.c_str(), &c);
		if (errno)
			return false;

		to = out;
		return true;
	}

	template <>
	inline bool deserialize<std::string>(std::string& to, NetReader& r)
	{
		to = r.readStringUntilNull(-1);
		return !r.isStringError();
	}

	template <>
	inline bool deserialize<glm::vec2>(glm::vec2& to, NetReader& r)
	{
		return r.get(to.x) && r.get(to.y);
	}

	template <>
	inline bool deserialize<glm::ivec2>(glm::ivec2& to, NetReader& r)
	{
		return r.get(to.x) && r.get(to.y);
	}

	template <typename T>
	bool deserializeArray(std::vector<T>& to, NetReader& r)
	{
		int size;
		if (!r.get(size))
			return false;
		to.reserve(size);
		to.resize(size);
		for (int i = 0; i < size; ++i) {
			if (!r.get(to[i]))
				return false;
		}

		// now this is spooky:
		// following version returns in if false instead of true
		// Boooo
		/*for (int i = 0; i < size; ++i) 
			if (!r.get(to[i]))
				return false;
		*/
		

		return true;
	}
}

// ring buffer with Random Access
class NetRingBuffer
{
	Buffer m_buffer;

	size_t m_consumer = 0;
	size_t m_producer = 0;

public:
	NetRingBuffer(size_t size = 0)
	{
		reserve(size);
	}

	void reserve(size_t size)
	{
		m_buffer.reserve(size);
		m_buffer.setSize(size);
	}

	// bytes available for read
	size_t available() const
	{
		return m_producer - m_consumer;
	}

	// bytes available for read from pointer
	size_t available(size_t pointer) const
	{
		if (m_producer < pointer)
			return 0;
		return m_producer - pointer;
	}

	// bytes available to write
	size_t freeSpace() const
	{
		return m_buffer.size() - available();
	}

	// bytes available to write starting from fromPointer
	size_t freeSpace(size_t fromPointer) const
	{
		if (fromPointer < m_producer)
			return 0;
		size_t avail = freeSpace();
		size_t requiredOffset = fromPointer - m_producer;
		if (avail < requiredOffset)
			return 0;
		return avail - requiredOffset;
	}

	// moves cursor, sending all data before it
	void seekp(size_t pointer)
	{
		ASSERT(pointer >= m_producer, "cannot move pointer back");
		m_producer = pointer;
	}

	// moves cursor, discarding all data before it
	void seekg(size_t pointer)
	{
		ASSERT(pointer >= m_consumer, "cannot move pointer back");
		m_consumer = pointer;
	}

	// writes at specified coordinates
	// !Does not advance writer pointer!
	// Use seekp() to advance the pointer
	size_t write(const char* data, size_t pointer, size_t len)
	{
		bool isFirstChunk = pointer == m_producer;

		size_t toWrite = std::min(len, freeSpace(pointer));
		if (toWrite == 0)
			return 0;

		size_t startIdx = pointer % m_buffer.size();
		size_t endIdx = (pointer + toWrite) % m_buffer.size();

		if (startIdx < endIdx)
		{
			memcpy(m_buffer.data() + startIdx, data, toWrite);
		}
		else
		{
			memcpy(m_buffer.data() + startIdx, data, m_buffer.size() - startIdx);
			memcpy(m_buffer.data(), data + m_buffer.size() - startIdx, endIdx);
		}
		return toWrite;
	}


	// automatically advances write pointer
	size_t write(const char* data, size_t len)
	{
		auto shift = write(data, m_producer, len);
		m_producer += shift; //wtf
		return shift;
	}

	// reads from specified coordinates
	// !Does not advance reader pointer!
	// Use seekg() to advance the pointer
	size_t read(char* data, size_t pointer, size_t maxLen)
	{
		size_t toRead = std::min(maxLen, available(pointer));

		size_t startIdx = pointer % m_buffer.size();
		size_t endIdx = (pointer + toRead) % m_buffer.size();

		if (startIdx < endIdx)
		{
			memcpy(data, m_buffer.data() + startIdx, toRead);
		}
		else
		{
			memcpy(data, m_buffer.data() + startIdx, m_buffer.size() - startIdx);
			memcpy(data + m_buffer.size() - startIdx, m_buffer.data(), endIdx);
		}
		return toRead;
	}

	// automatically advances read pointer
	size_t read(char* data, size_t maxLen)
	{
		auto shift = read(data, m_consumer, maxLen);
		m_consumer += shift;
		return shift;
	}

	size_t tellg() { return m_consumer; }
	size_t tellp() { return m_producer; }
};
}
