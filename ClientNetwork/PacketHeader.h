#pragma once

namespace NetHelper
{
#pragma pack (push, 1)
	struct PacketHeader
	{
		uint16 pkt_size;
		uint16 pkt_id;
	};
#pragma pack (pop)
}