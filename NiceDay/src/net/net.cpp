#include "net.h"
#include "ndpch.h"
#include "net.h"

#include <random>
#ifdef ND_PLATFORM_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <WS2spi.h>
// need link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#else
#endif


namespace nd::net
{
thread_local std::vector<Socket> s_sockets;

Address::Address(const std::string& ip, int port)

{
	src = {};
	src.sin_family = AF_INET; /* Internet/IP */
	src.sin_addr.s_addr = inet_addr(ip.c_str()); /* Any IP address */
	src.sin_port = htons(port); /* server port */
}

Address::Address(unsigned long ip, int port)
	: Address()
{
	src = {};
	src.sin_family = AF_INET; /* Internet/IP */
	src.sin_addr.s_addr = htonl(ip); /* Any IP address */
	src.sin_port = htons(port); /* server port */
}


bool Address::isValid() const
{
#ifdef ND_PLATFORM_WINDOWS
	return src.sin_addr.s_addr != ((ULONG)-1) && ntohs(src.sin_port) <= 65535;
#else
	return src.sin_addr.s_addr != ((in_addr_t)-1) && ntohs(src.sin_port) <= 65535;
#endif
}

Address Address::build(const std::string& ipWithPort)
{
	std::vector<std::string> split;
	SUtil::splitString(ipWithPort, split, ":");
	ASSERT(split.size() == 2, "invalid address");

	return {split[0], std::stoi(split[1])};
}

std::string Address::toString() const
{
	//char str[INET_ADDRSTRLEN];

	// now get it back and print it
	//inet_ntop(AF_INET, &src, str, INET_ADDRSTRLEN);

	return ip() + ":" + std::to_string(port());
}

int Address::port() const
{
	return ntohs(src.sin_port);
}

std::string Address::ip() const
{
#ifdef ND_PLATFORM_WINDOWS
	std::stringstream s;
	s << (int)src.sin_addr.S_un.S_un_b.s_b1;
	s << ".";
	s << (int)src.sin_addr.S_un.S_un_b.s_b2;
	s << ".";
	s << (int)src.sin_addr.S_un.S_un_b.s_b3;
	s << ".";
	s << (int)src.sin_addr.S_un.S_un_b.s_b4;
	return s.str();
#else
	char buf[20];
	return inet_ntop(AF_INET, &src.sin_addr, buf, sizeof(buf));
#endif
}

NetResponseFlags_ init()
{
#ifdef ND_PLATFORM_WINDOWS

	WSADATA wsa;
	//Initialise winsock
	ND_TRACE("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		ND_ERROR("Failed. Error Code :{}", WSAGetLastError());
		return NetResponseFlags_Error;
	}
	ND_TRACE("winsock Initialised.\n");

#endif
	return NetResponseFlags_Success;
}

void deInit()
{
	while (!s_sockets.empty())
		closeSocket(s_sockets[0]);
}

static int findSocket(const Socket& s)
{
	for (int i = 0; i < s_sockets.size(); ++i)
		if (s_sockets[i] == s)
			return i;
	return -1;
}

NetResponseFlags_ createSocket(Socket& s, const CreateSocketInfo& info)
{
	/* Create the UDP socket */
	if ((s.m_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		ND_ERROR("Cannot create socket, make sure to call nd::net::init() before creating any socket.");
		return NetResponseFlags_Error;
	}

	/* Construct the server sockaddr_in structure */
	s.m_address = Address(INADDR_ANY, info.port);

	/* Bind the socket */
	if (bind(s.m_sock, (sockaddr*)&s.m_address.src, sizeof(s.m_address)) < 0)
	{
		ND_ERROR("Cannot bind socket {}", info.port);
		return NetResponseFlags_Error;
	}

	//enable async
	{
		u_long enabled = info.async;
#ifdef ND_PLATFORM_WINDOWS
		ioctlsocket(s.m_sock, FIONBIO, &enabled);
#else
		int flags = fcntl(s.m_sock, F_GETFL, 0);
		if (flags != -1)
			flags = !info.async ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		fcntl(s.m_sock, F_SETFL, flags) == 0;
#endif
	}

	ND_INFO("Socket opened on port {}", info.port);
	s_sockets.push_back(s);
	return NetResponseFlags_Success;
}


NetResponseFlags_ closeSocket(Socket& s)
{
	int socketid = findSocket(s);
	if (socketid == -1)
	{
		ND_WARN("Attempt to close non existing socket");
		return NetResponseFlags_Error;
	}
#ifdef ND_PLATFORM_WINDOWS
	closesocket(s.m_sock);
#else
	close(s.m_sock);
#endif
	s_sockets.erase(s_sockets.begin() + socketid);
	return NetResponseFlags_Success;
}

#ifdef ND_PLATFORM_WINDOWS
typedef int sock_address_size;
#else
typedef socklen_t sock_address_size;
#endif


NetResponseFlags_ receive(Socket& socket, Message& m)
{
	sock_address_size SenderAddrSize = sizeof(m.address.src);

	int received = recvfrom(socket.m_sock, m.buffer.data(), m.buffer.capacity(), 0,
	                        (sockaddr*)&m.address.src, &SenderAddrSize);

	if constexpr (SIMULATE_PACKET_LOSS != 0)
	{
		static thread_local std::mt19937 generator;
		std::uniform_int_distribution<int> distribution(0, SIMULATE_PACKET_LOSS);


		if (received != -1)
		{
			if (!(distribution(generator) % SIMULATE_PACKET_LOSS))
			{
				return NetResponseFlags_Error;
			}
		}
	}

	if (received != -1 /* || received != ERROR_TIMEOUT*/)
	{
		m.buffer.setSize(received);
		socket.m_received_count++;
		socket.m_received_bytes += m.buffer.size();
		return NetResponseFlags_Success;
	}
	return NetResponseFlags_Error;
}

NetResponseFlags_ send(Socket& s, const Message& m)
{
	if (sendto(s.m_sock, m.buffer.data(), m.buffer.size(), 0, (const sockaddr*)&m.address.src,
	           sizeof(m.address.src)) == -1)
	{
		ND_WARN("Error during sending to address {}", m.address.toString());
		//ASSERT(false, "Error during sending to address {}", m.address.toString());
		return NetResponseFlags_Error;
	}
	s.m_sent_count++;
	s.m_sent_bytes += m.buffer.size();
	return NetResponseFlags_Success;
}
}
