#include "ClientNetworkPch.h"
#include "SendBufferChunk.h"
#include "SendBuffer.h"

namespace NetHelper
{
	SendBufferChunk::SendBufferChunk()noexcept
	{
	}

	SendBufferChunk::~SendBufferChunk()noexcept
	{
	}

	S_ptr<SendBuffer> SendBufferChunk::Open(const S_ptr<SendBufferChunk>& pCurrentChunkBuffer, c_uint32 allocSize)noexcept
	{
		NET_NAGOX_ASSERT(allocSize <= SEND_BUFFER_CHUNK_SIZE);
		NET_NAGOX_ASSERT(false == m_bOpen);

		m_bOpen = true;

		return std::make_shared<SendBuffer>(pCurrentChunkBuffer, Buffer(), allocSize);
	}
}
