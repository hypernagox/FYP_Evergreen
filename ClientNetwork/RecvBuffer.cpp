#include "ClientNetworkPch.h"
#include "RecvBuffer.h"

namespace NetHelper
{
	RecvBuffer::RecvBuffer(c_int32 bufferSize_)noexcept
		: m_bufferSize{ bufferSize_ }
		, m_capacity{ bufferSize_ * BUFFER_COUNT }
	{

	}

	RecvBuffer::~RecvBuffer()
	{
	}

	void RecvBuffer::Clear()noexcept
	{
		const int32 dataSize = DataSize();
		if (0 == dataSize)
		{
			m_readPos = m_writePos = 0;
		}
		else
		{
			if (FreeSize() < m_bufferSize)
			{
				::memcpy(m_buffer, m_buffer + m_readPos, dataSize);
				m_readPos = 0;
				m_writePos = dataSize;
			}
		}
	}
}