#include "pch.h"
#include "DBPacket.h"
#include "s2q_PacketHandler.h"
#include "DBMgr.h"
#include "DBBindRAII.h"

void AddProtocol(const uint8 pktID_, const PacketHandlerFunc fpPacketHandler_) noexcept
{
	s2q_PacketHandler::AddProtocol(pktID_, fpPacketHandler_);
}

const bool s2q_PLAYER_MOVE::Handle(s2q_PLAYER_MOVE& pkt_)
{
	DBBindRAII<3, 0> dbBinder{ L"UPDATE [dbo].[playerInfo] SET x = ?, y = ? WHERE playerId = ?" };
	dbBinder.BindParam(0, pkt_.x);
	dbBinder.BindParam(1, pkt_.y);
	dbBinder.BindParam(2, pkt_.name);

	dbBinder.Execute();

	return false;
}
