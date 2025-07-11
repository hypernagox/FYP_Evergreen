#include "pch.h"
#include "s2q_PacketHandler.h"

void Handle_Invalid(const char* const pBuff_)
{
}

void AddProtocol(const uint16_t pktID_, void(*fpPacketHandler_)(const char* const)) noexcept
{
	s2q_PacketHandler::AddProtocol(pktID_, fpPacketHandler_);
}
