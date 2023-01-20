#pragma once
#include "ndpch.h"
#ifdef ND_PLATFORM_WINDOWS
#include <ws2def.h>
#else
#include<arpa/inet.h>
#include<sys/socket.h>
#endif


enum NetResponseFlags_
{
	NetResponseFlags_Success,
	NetResponseFlags_Error,
	NetResponseFlags_Timeout,
};

namespace nd::net
{
constexpr char TERMINATOR = '#';
constexpr size_t SIMULATE_PACKET_LOSS = 0;

typedef int SocketID;

struct CreateSocketInfo
{
	uint32_t port;
	bool async;
};

struct Address
{
	sockaddr_in src;
	Address() = default;
	Address(const std::string& ip, int port);
	Address(unsigned long ip, int port);
	bool isValid() const;
	bool isPortReserved() const { return port() >= 0 && port() <= 1023; }
	static Address build(const std::string& ipWithPort);
	std::string toString() const;
	int port() const;
	std::string ip() const;

	bool operator==(const Address& address) const
	{
		return toString() == address.toString();
	}

	bool operator!=(const Address& address) const { return !this->operator==(address); }
};

struct Socket
{
	SocketID m_sock;
	Address m_address;

	int m_received_count = 0;
	int m_sent_count = 0;
	int m_received_bytes = 0;
	int m_sent_bytes = 0;

	bool operator==(std::vector<Socket>::const_reference socket) const
	{
		return m_sock == socket.m_sock;
	}
};


struct Buffer
{
	std::vector<char> buff;
	int siz = 0;

	void reserve(size_t size)
	{
		if (buff.size() < size)
			buff.resize(size);
		siz = 0;
	}

	void setSize(size_t size)
	{
		ASSERT(size <= capacity(), "invalid size");
		siz = size;
	}

	auto data() { return buff.data(); }
	auto data() const { return buff.data(); }
	int size() const { return siz; }
	int capacity() const { return (int)buff.size(); }
};

struct BufferWriter
{
	Buffer& b;
	int pointer;
	bool expand;

	BufferWriter(Buffer& b, bool append = true, bool expand = true): b(b), pointer(b.size()), expand(expand)
	{
		if (!append)
		{
			b.setSize(0);
			pointer = 0;
		}
	}

	template <typename T>
	void write(const T* t, int count)
	{
		const int sizeOfT = count * sizeof(T);

		ASSERT(expand || pointer + sizeOfT <= b.capacity(), "not enough space");
		if (expand && pointer + sizeOfT > b.capacity())
			b.reserve((pointer + sizeOfT) * 2);

		auto p = b.data() + pointer;

		memcpy(p, t, sizeOfT);

		pointer += sizeOfT;
		b.setSize(pointer);
	}

	template <typename T>
	void write(const T& t)
	{
		if constexpr (std::is_same_v<std::remove_const_t<T>, char*>)
		{
			write(std::to_string(t));
			return;
		}
		ASSERT(expand || pointer + sizeof(T) <= b.capacity(), "not enough space");
		if (expand && pointer + sizeof(T) > b.capacity())
			b.reserve((pointer + sizeof(T)) * 2);

		auto p = b.data() + pointer;

		memcpy(p, &t, sizeof(T));

		pointer += sizeof(T);
		b.setSize(pointer);
	}
};

template <>
inline void BufferWriter::write<std::string>(const std::string& tt)
{
	std::string what;
	what += TERMINATOR;
	std::string with;
	with += TERMINATOR;
	with += TERMINATOR;

	std::string t = tt;
	SUtil::replaceWith(t, what.c_str(), with.c_str());

	if (t.empty())
		t = " ";


	ASSERT(expand || pointer + t.size() <= b.capacity(), "not enough space");
	if (expand && pointer + t.size() > b.capacity())
		b.reserve((pointer + t.size()) * 2);

	auto p = b.data() + pointer;

	memcpy(p, t.c_str(), t.size());
	pointer += t.size();

	if (pointer < b.capacity())
		b.data()[pointer++] = TERMINATOR;

	b.setSize(pointer);
}

template <>
inline void BufferWriter::write<const char*>(const char* const& t)
{
	write(std::string(t));
}


struct BufferReader
{
	Buffer& b;
	int pointer;

	BufferReader(Buffer& b) : b(b), pointer(0)
	{
	}

	template <typename T>
	void read(T& t)
	{
		ASSERT(pointer + sizeof(T) <= b.size(), "not enough space");
		memcpy(&t, b.data() + pointer, sizeof(T));

		pointer += sizeof(T);
	}

	template <typename T>
	T* tryRead()
	{
		if (pointer + sizeof(T) > b.size())
			return nullptr;

		auto out = reinterpret_cast<T*>(b.data() + pointer);
		pointer += sizeof(T);
		return out;
	}

private:
	bool string_error;
public:
	// true if readString could not read another string
	// empty string is not an error, no more space in buffer is
	bool isStringError() const { return string_error; }

	int strln(char const* c, int maxCount)
	{
		int count = 0;
		while (true)
		{
			while (*(c++) != TERMINATOR && count++ < maxCount);

			if (count + 1 >= maxCount || (*c) != TERMINATOR)
				break;

			c++; //move from the next terminator
			count += 2; //eat 2 terminators
		}
		// terminator missing
		if (*(c - 1) != TERMINATOR)
			return -1;
		return count;
	}

	std::string readStringUntilNull(int maxSize)
	{
		string_error = false;

		if (maxSize == -1)
			maxSize = std::numeric_limits<int>::max();

		auto remaingSize = std::min(maxSize, b.size() - pointer);

		if (remaingSize <= 0)
		{
			string_error = true;
			return "";
		}

		auto out = b.data() + pointer;

#ifdef NOT_EX
#ifdef ND_PLATFORM_WINDOWS
		auto length = strnlen_s(b.data() + pointer, remaingSize);
#else
		auto length = strnlen(b.data() + pointer, remaingSize);
#endif
#endif
		auto length = strln(b.data() + pointer, remaingSize);
		if(length==-1)
		{
			string_error = true;
			return "";
		}
		pointer += length;

		//add one to discard null character as well
		pointer += pointer < b.size() && *(b.data() + pointer) == TERMINATOR;

		std::string what;
		what += TERMINATOR;
		what += TERMINATOR;
		std::string with;
		with += TERMINATOR;

		auto o = std::string(out, length);

		SUtil::replaceWith(o, what.c_str(), with.c_str());
		if (o == " ")
			return "";
		return o;
	}
};

struct Message
{
	Address address;
	Buffer buffer;
};

// must be called before any socket madness is used by program
NetResponseFlags_ init();

// closes all sockets opened by current thread
void deInit();
NetResponseFlags_ createSocket(Socket&, const CreateSocketInfo&);
NetResponseFlags_ closeSocket(Socket&);
NetResponseFlags_ receive(Socket&, Message&);
NetResponseFlags_ send(Socket&, const Message&);
};
