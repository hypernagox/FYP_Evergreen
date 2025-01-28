#include "ServerCorePch.h"
#include "SendBufferChunk.h"
#include "SendBuffer.h"
#include "ObjectPool.hpp"

namespace ServerCore
{
	S_ptr<SendBuffer> SendBufferChunk::Open(c_uint32 allocSize)noexcept
	{
		static_assert(UINT16_MAX > sizeof(m_buffer));
		static_assert((UINT16_MAX + 1) == sizeof(SendBufferChunk));
		NAGOX_ASSERT(allocSize <= SEND_BUFFER_CHUNK_SIZE);
		NAGOX_ASSERT(false == m_bOpen);

		m_bOpen = true;

		return MakeShared<SendBuffer>(S_ptr<SendBufferChunk>{this}, Buffer(), allocSize);
	}
}
