#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
#pragma pack (push, 1)
	struct PacketHeader
	{
		uint16 pkt_size;
		uint16 pkt_id;
	};
#pragma pack (pop)

	struct RecvStatus
	{
		c_int32 processLen;
		const bool bIsOK;
	};
}