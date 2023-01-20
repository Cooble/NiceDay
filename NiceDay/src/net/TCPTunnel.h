#pragma once
#include "ndpch.h"
#include "net.h"
#include "net_iterator.h"

namespace nd::net
{
typedef int ClientID;

constexpr int TCP_PACKET_FAT_ID = -1;
constexpr int TCP_PACKET_ACK_ACTION = -2;

struct SegmentPacket : Serializable
{
	int action;
	ClientID channelId;
	int packetId;
	int memoryIdx;

	bool deserialize(NetReader& reader)
	{
		if (!reader.get(action))
			return false;
		if (action != TCP_PACKET_FAT_ID)
			return false;

		static std::string bang = "TCP";
		static std::string e;
		reader.get(e, 4);
		if (e != "TCP")
			return false;
		return
			reader.get(channelId) &&
			reader.get(packetId) &&
			reader.get(memoryIdx);
	}

	void serialize(NetWriter& writer) const
	{
		writer.put(TCP_PACKET_FAT_ID);
		static std::string bang = "TCP";
		writer.put(bang);
		writer.put(channelId);
		writer.put(packetId);
		writer.put(memoryIdx);
	}
};

struct AckPacket : Serializable
{
	int action;
	ClientID clientId;
	std::vector<size_t> ack_ids;
	std::vector<size_t> nack_ids;

	bool isEmpty() const { return ack_ids.empty() && nack_ids.empty(); }

	void clear()
	{
		ack_ids.clear();
		nack_ids.clear();
	}

	bool deserialize(NetReader& reader)
	{
		if (!reader.get(action))
			return false;
		if (action != TCP_PACKET_ACK_ACTION)
			return false;

		static std::string bang = "TCP_ACK";
		std::string e;
		reader.get(e, bang.size());
		if (e != "TCP_ACK")
			return false;

		return reader.get(clientId) && reader.get(ack_ids) && reader.get(nack_ids);
	}

	void serialize(NetWriter& writer) const
	{
		writer.put(TCP_PACKET_ACK_ACTION);
		static std::string bang = "TCP_ACK";
		writer.put(bang);
		writer.put(clientId);
		writer.put(ack_ids);
		writer.put(nack_ids);
	}
};

struct Pac
{
	size_t packetId;
	size_t memoryIdx;
	size_t length = 0;
	size_t timeout = 0;

	bool isAbsent() const { return !length; }

	bool isAck = false;

	size_t nextMemoryIdx() const { return memoryIdx + length; }
	void acknowledge() { isAck = true; }
};

class TCPTunnel
{
public:
	static constexpr size_t TCP_TUNNEL_TIMEOUT_MS = 100;
	static constexpr int PACKETS_IN_WINDOW = 50;
	//static constexpr size_t MAX_UDP_PACKET_LENGTH = 1024 + 100; //dont forget to include sizeof(FatPacket)
	static constexpr size_t MAX_UDP_PACKET_LENGTH = 1000; //dont forget to include sizeof(FatPacket)

	Socket& m_socket;
	ClientID m_channel_id;
	NetRingBuffer m_buff_in, m_buff_out;
	std::vector<Pac> m_in_window;
	std::vector<Pac> m_out_window;

	Pac m_current_write;
	AckPacket m_current_ack;
	Address m_address;

	bool m_err=false;

public:
	TCPTunnel(Socket& socket, Address a, ClientID id, size_t buffSize) :
		m_socket(socket),
		m_channel_id(id),
		m_buff_in(buffSize),
		m_buff_out(buffSize),
		m_address(a)

	{
		m_current_ack.clientId = m_channel_id;
		m_current_write = {0, 0, 0};

		for (size_t i = 0; i < PACKETS_IN_WINDOW; ++i)
		{
			Pac p = {i, 0, 0};
			m_in_window.push_back(p);
			m_out_window.push_back(p);
		}
	}

	// returns client id of a message or -1 if message is not tcp
	static ClientID getClientID(Message& m)
	{
		SegmentPacket h;
		if (NetReader(m.buffer).get(h))
			return h.channelId;

		AckPacket p;
		if (NetReader(m.buffer).get(p))
			return p.clientId;

		return -1;
	}


	// receive udp packet
	// before calling you have to check that getClientID() returns id of this tunnel
	void receiveTunnelUDP(Message& m);

	// sends buffered udp packets
	// content of message is not relevant
	void flushTunnelUDP(Message& m);

	// flush current ack packet
	void flushAck(Message& m);

	//flush everything
	void flush(Message& m)
	{
		flushAck(m);
		flushTunnelUDP(m);
	}

	bool write(const Message& m);
	bool read(Message& m);

	bool isError()const { return m_err; }
private:
	void receiveFatPacket(Message& m);
	void receiveAckPacket(Message& m);

	auto& waitingForFirst() { return m_in_window[0]; }
	// send packets that timed out
	void flushTimeouts(Message& m);
	void setError() { m_err = true; }

};
}
