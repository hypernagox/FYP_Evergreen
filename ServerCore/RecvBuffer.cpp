#include "ServerCorePch.h"
#include "RecvBuffer.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
{
	RecvBuffer::RecvBuffer(c_int32 bufferSize_)noexcept
		: m_bufferSize{ (uint16_t)bufferSize_ }
		, m_capacity{ (uint16_t)(bufferSize_ * BUFFER_COUNT) }
		, m_readPos{ 0 }
		, m_writePos{ 0 }
	{
		static_assert(UINT16_MAX > sizeof(m_buffer));
		static_assert((UINT16_MAX + 1) == sizeof(RecvBuffer));
	}

	void RecvBuffer::Clear()noexcept
	{
		if (const int32 dataSize = DataSize())
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
