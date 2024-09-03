#include "ServerCorePch.h"
#include "PacketSession.h"

namespace ServerCore
{
	PacketSession::PacketSession(const PacketHandleFunc* const sessionPacketHandler_)noexcept
		:Session{ sessionPacketHandler_ }
	{
	}

	PacketSession::PacketSession(const PacketHandleFunc* const sessionPacketHandler_, const bool bNeedConnect) noexcept
		:Session{ sessionPacketHandler_ ,bNeedConnect }
	{
	}

	const RecvStatus PacketSession::OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept
	{
		const PacketHandleFunc* const __restrict packet_handler = m_sessionPacketHandler;
		int32 processLen = 0;
		bool bIsOk = true;
		for (;;)
		{
			const int32 dataSize = len - processLen; // 두번째부터는 이미 해놓은게 있기 때문
			// 최소한 헤더는 파싱할 수 있는가?
			if (dataSize < static_cast<c_int32>(sizeof(PacketHeader)))
				break;

			const PacketHeader* const __restrict header = reinterpret_cast<const PacketHeader* const>(buffer + processLen);
			c_int32 packetSize = header->pkt_size;
			c_uint16 packetId = header->pkt_id;
			// 헤더에 기록된 패킷크기를 파싱할수있나?
			if (dataSize < packetSize)
				break;

			// 일단 오긴다옴, 조립성공

			if (static_cast<c_uint16>(HEART_BEAT::c2s_HEART_BEAT) == packetId)
			{
				SetHeartBeat(true);
			}
			else
			{
				bIsOk &= packet_handler[packetId](pThisSessionPtr, reinterpret_cast<const BYTE* const>(header), packetSize);
			}

			processLen += packetSize;
		}

		return { processLen,bIsOk };
	}
}