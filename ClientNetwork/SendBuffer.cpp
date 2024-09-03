#include "ClientNetworkPch.h"
#include "SendBuffer.h"
#include "SendBufferChunk.h"

namespace NetHelper
{
	SendBuffer::SendBuffer(S_ptr<SendBufferChunk> owner, BYTE* const buffer, c_uint32 allocSize_)noexcept
		: m_pOwnerChunk{ std::move(owner) }
		, m_buffer{ buffer }
		, m_allocSize{ allocSize_ }
		, m_writeSize{ 0 }
	{
	}

	SendBuffer::~SendBuffer()noexcept
	{
	}

	void SendBuffer::Close(c_uint32 writeSize_)noexcept
	{
		NET_NAGOX_ASSERT(m_allocSize >= writeSize_);
		m_writeSize = writeSize_;
		m_pOwnerChunk->Close(writeSize_);
	}
}