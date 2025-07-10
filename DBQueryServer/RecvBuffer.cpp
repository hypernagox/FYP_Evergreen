#include "pch.h"
#include "RecvBuffer.h"


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
		if (FreeSize() < static_cast<c_int32>(RECV_BUFFER_SIZE::BUFFER_SIZE >> 1))
		{
			::memcpy(m_buffer, m_buffer + m_readPos, dataSize);
			m_readPos = 0;
			m_writePos = dataSize;
		}
	}
}
