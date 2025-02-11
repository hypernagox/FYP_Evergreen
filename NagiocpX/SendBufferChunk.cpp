#include "NagiocpXPch.h"
#include "SendBufferChunk.h"
#include "SendBuffer.h"
#include "ObjectPool.hpp"

namespace NagiocpX
{
	S_ptr<SendBuffer> SendBufferChunk::Open(c_uint32 allocSize)noexcept
	{
		static_assert(0 == SEND_BUFFER_CHUNK_SIZE % (PAGE_SIZE * 16));
		static_assert(0 == SEND_BUFFER_CHUNK_SIZE % PAGE_SIZE);
		NAGOX_ASSERT(allocSize <= SEND_BUFFER_CHUNK_SIZE);
		NAGOX_ASSERT(false == m_bOpen);

		m_bOpen = true;

		if (PAGE_SIZE >= allocSize) [[likely]]
		{
			const auto old_page = m_curPage;
			m_curPage = ((m_usedSize + allocSize - 1) >> 12);
			m_usedSize += (~(m_curPage - old_page - 1)) & ((m_curPage << 12) - m_usedSize);
		}

		return MakeShared<SendBuffer>(this, Buffer(), allocSize);
	}
}
