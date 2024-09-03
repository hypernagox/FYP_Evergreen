#pragma once

/*--------------
	SendBufferChunk
---------------*/

namespace ServerCore
{
	class SendBuffer;

	class SendBufferChunk
		:public RefCountable
	{
		friend class SendBufferMgr;
		enum
		{
			SEND_BUFFER_CHUNK_SIZE = 0x10000 - 32
		};
	public:
		SendBufferChunk()noexcept = default;
		~SendBufferChunk()noexcept = default;
		void Close(c_uint32 writeSize_)noexcept
		{
			NAGOX_ASSERT(true == m_bOpen);
			m_bOpen = false;
			m_usedSize += writeSize_;
		}
	private:
		S_ptr<SendBuffer> Open(c_uint32 allocSize)noexcept;
		void Reset()noexcept { m_usedSize = m_bOpen = false; }
		const bool IsOpen()const noexcept { return m_bOpen; }
		BYTE* const Buffer()noexcept { return m_buffer + m_usedSize; }
		c_uint32 FreeSize()const noexcept { return SEND_BUFFER_CHUNK_SIZE - m_usedSize; }
	private:
		bool m_bOpen = false;
		uint32 m_usedSize = 0;
		BYTE m_buffer[SEND_BUFFER_CHUNK_SIZE];
	};
}
