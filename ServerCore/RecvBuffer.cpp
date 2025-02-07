#include "ServerCorePch.h"
#include "RecvBuffer.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
{
	RecvBuffer::RecvBuffer()noexcept
		: m_readPos{ 0 }
		, m_writePos{ 0 }
	{
		static_assert(0 == RECV_BUF_SIZE % (PAGE_SIZE * 16));
		static_assert(0 == RECV_BUF_SIZE % PAGE_SIZE);
	}

	void RecvBuffer::Clear()noexcept
	{
		constexpr const int32_t HALF_SLOT = (BUF_SLOT_SIZE / 2);
		if (const int32_t dataSize = DataSize())
		{
			if (FreeSize() < HALF_SLOT)
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
