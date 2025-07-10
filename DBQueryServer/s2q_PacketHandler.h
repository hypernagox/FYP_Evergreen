#pragma once
#include "pch.h"
#include "DBPacket.h"


const bool Handle_Invalid(BYTE* const pBuff_, c_int32 len_);

class s2q_PacketHandler
{
public:
	static inline PacketHandlerFunc g_fpPacketHandler[UINT8_MAX] = {};

	static void Init() noexcept
	{
		for (auto& fpHandlerFunc : g_fpPacketHandler)
		{
			if (nullptr == fpHandlerFunc)
				fpHandlerFunc = Handle_Invalid;
		}
	}

	static const PacketHandlerFunc* const GetPacketHandlerList()noexcept { return g_fpPacketHandler; }

	static void AddProtocol(const uint8 pktID_, const PacketHandlerFunc fpPacketHandler_)noexcept
	{
		NAGOX_ASSERT(nullptr == g_fpPacketHandler[pktID_] || Handle_Invalid == g_fpPacketHandler[pktID_]);
		g_fpPacketHandler[pktID_] = fpPacketHandler_;
	}

	static const bool HandlePacket(BYTE* const pBuff_, c_int32 len_)noexcept
	{
		const DBPacketHeader* const header = reinterpret_cast<const DBPacketHeader* const>(pBuff_);
		return g_fpPacketHandler[header->pkt_id](pBuff_, len_);
	}
public:
	s2q_PacketHandler() = delete;
	s2q_PacketHandler(const s2q_PacketHandler&) = delete;
	s2q_PacketHandler(s2q_PacketHandler&&) = delete;
	s2q_PacketHandler& operator=(const s2q_PacketHandler&) = delete;
	s2q_PacketHandler& operator=(s2q_PacketHandler&&) = delete;
	~s2q_PacketHandler() = delete;
};