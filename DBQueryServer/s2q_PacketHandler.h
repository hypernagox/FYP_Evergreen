#pragma once
#include "pch.h"
#include "../DBContentsPacket.hpp"


void Handle_Invalid(const char* const pBuff_);

class s2q_PacketHandler
{
public:
	using PacketHandlerFunc = void(*)(const char* const);

	static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};

	static void Init() noexcept
	{
		for (auto& fpHandlerFunc : g_fpPacketHandler)
		{
			if (nullptr == fpHandlerFunc)
				fpHandlerFunc = Handle_Invalid;
		}
	}

	static const PacketHandlerFunc* const GetPacketHandlerList()noexcept { return g_fpPacketHandler; }

	static void AddProtocol(const uint16 pktID_, const PacketHandlerFunc fpPacketHandler_)noexcept
	{
		NAGOX_ASSERT(nullptr == g_fpPacketHandler[pktID_] || Handle_Invalid == g_fpPacketHandler[pktID_]);
		g_fpPacketHandler[pktID_] = fpPacketHandler_;
	}

	static void HandlePacket(const char* const pBuff_, c_int32 len_)noexcept
	{
		const DBContentsPacketHeader* const header = reinterpret_cast<const DBContentsPacketHeader* const>(pBuff_);
		return g_fpPacketHandler[header->pkt_id](pBuff_);
	}
public:
	s2q_PacketHandler() = delete;
	s2q_PacketHandler(const s2q_PacketHandler&) = delete;
	s2q_PacketHandler(s2q_PacketHandler&&) = delete;
	s2q_PacketHandler& operator=(const s2q_PacketHandler&) = delete;
	s2q_PacketHandler& operator=(s2q_PacketHandler&&) = delete;
	~s2q_PacketHandler() = delete;
};

void AddProtocol(const uint16_t pktID_, void (*fpPacketHandler_)(const char* const))noexcept;