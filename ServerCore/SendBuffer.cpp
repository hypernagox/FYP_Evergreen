#include "ServerCorePch.h"
#include "SendBuffer.h"
#include "SendBufferChunk.h"

namespace ServerCore
{
	void SendBuffer::Close(c_uint32 writeSize_)noexcept
	{
		NAGOX_ASSERT(m_allocSize >= writeSize_);
		m_writeSize = writeSize_;
		m_pOwnerChunk->Close(writeSize_);
	}
}