#include "ServerCorePch.h"
#include "RecvBuffer.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
{
	RecvBuffer::RecvBuffer()noexcept
		: m_readPos{ 0 }
		, m_writePos{ 0 }
	{
		static_assert(UINT16_MAX > sizeof(m_buffer));
		static_assert((UINT16_MAX + 1) == sizeof(RecvBuffer));
		static_assert(sizeof(RecvBuffer) == MEMBER_SIZE_SUM(RecvBuffer, m_buffer));
		static_assert(sizeof(RecvBuffer) == DESIRE_BUF_SIZE);
		static_assert(DESIRE_BUF_SIZE == MEMBER_SIZE_SUM(RecvBuffer, m_buffer));
	}

	void RecvBuffer::Clear()noexcept
	{
		if (const int32_t dataSize = DataSize())
		{
			if (FreeSize() < static_cast<c_int32>(RECV_BUFFER_SIZE::BUFFER_SIZE >> 1))
			{
				::memcpy(m_buffer, m_buffer + m_readPos, dataSize);
				m_readPos = 0;
				m_writePos = dataSize;
			}
		}
		else
		{
			m_readPos = m_writePos = 0;
		}
	}
}
