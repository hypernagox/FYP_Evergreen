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

			// ��Ŷ�� ���Ե� ������� �ڵ鸵 �� ��Ŷ�� ���� ����� ��ġ�ϴ� �� üũ�� �ݵ�� �ʿ�
			// ������ �ܿ��� ȣ���� ��, �ڵ鸵 �� ��Ŷ������� ���� ����� ������ ���θ� üũ�ϴ� �κ��� �־����
			// ��Ŷ����� 0�� �� ���ѷ����� ����.
			if (!packet_handler[packetId](pThisSessionPtr, reinterpret_cast<const BYTE* const>(header), packetSize))
				return { processLen,false };
			// ���߿� �� �Լ��� �����ڵ带 ��ȯ�ϰ�, ������ ��Ʈ������ �����ؼ� ����ϴ� ������ ����ó���� �Ѵ�.
		} 

		return { processLen,true };
	}
}