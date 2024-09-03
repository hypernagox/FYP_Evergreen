#include "ServerCorePch.h"
#include "RecvBuffer.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
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
			// 리드커서와 라이트커서가 딱 겹치는 상황 (알맞게 잘 읽음) 그냥 0으로 둘이 보낸다.
			m_readPos = m_writePos = 0;
		}
		else
		{
			// 어쩔 수 없이 앞으로 땡기는 복사비용 발생
			// 여유 공간이 버퍼 1개 크기 미만이면 당김
			if (FreeSize() < static_cast<c_int32>(RECV_BUFFER_SIZE::BUFFER_SIZE >> 1))
			{
				::memcpy(m_buffer, m_buffer + m_readPos, dataSize);
				m_readPos = 0;
				m_writePos = dataSize;
			}
		}
	}
}
