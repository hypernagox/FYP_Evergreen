#include "ServerCorePch.h"
#include "PacketSession.h"

namespace ServerCore
{
	PacketSession::PacketSession(const bool bNeedConnect) noexcept
		:Session{ bNeedConnect }
	{
	}

	const RecvStatus PacketSession::OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept
	{
		const PacketHandleFunc* const __restrict packet_handler = g_sessionPacketHandler;
		int32_t processLen = 0;

		for(;;) [[likely]]
		{
			const PacketHeader* const __restrict header = reinterpret_cast<const PacketHeader* const>(buffer + processLen);
			const int32_t dataSize = len - processLen;

			if (dataSize < static_cast<c_int32>(sizeof(PacketHeader)))
				break;

			c_int32 packetSize = header->pkt_size;
			c_uint16 packetId = header->pkt_id;

			if (dataSize < packetSize)
				break;

			processLen += packetSize;

			// 패킷에 기입된 사이즈와 핸들링 할 패킷의 실제 사이즈가 일치하는 지 체크가 반드시 필요
			// 컨텐츠 단에서 호출할 때, 핸들링 할 패킷사이즈와 들어온 사이즈가 같은지 여부를 체크하는 부분이 있어야함
			// 패킷사이즈가 0일 때 무한루프가 돈다.
			if (!packet_handler[packetId](pThisSessionPtr, reinterpret_cast<const BYTE* const>(header), packetSize))
				return { processLen,false };
			// 나중에 이 함수는 에러코드를 반환하고, 세션의 라스트에러에 저장해서 기록하는 식으로 에러처리를 한다.
		} 

		return { processLen,true };
	}
}