#pragma once

namespace NetHelper
{
	class SendBuffer;

	class SendBufferChunk
	{
		enum
		{
			SEND_BUFFER_CHUNK_SIZE = 0x1000 * 2
		};
	public:
		SendBufferChunk()noexcept;
		~SendBufferChunk()noexcept;
		S_ptr<SendBuffer> Open(const S_ptr<SendBufferChunk>& pCurrentChunkBuffer, c_uint32 allocSize)noexcept;

		void Reset()noexcept
		{
			m_bOpen = false;
			m_usedSize = 0;
		}

		void Close(c_uint32 writeSize_)noexcept
		{
			NET_NAGOX_ASSERT(true == m_bOpen);
			m_bOpen = false;
			m_usedSize += writeSize_;
		}

		const bool IsOpen()const noexcept { return m_bOpen; }
		BYTE* const Buffer()noexcept { return m_buffer + m_usedSize; }
		c_uint32 FreeSize()const noexcept { return SEND_BUFFER_CHUNK_SIZE - m_usedSize; }

	private:
		bool m_bOpen = false;
		uint32 m_usedSize = 0;
		BYTE m_buffer[SEND_BUFFER_CHUNK_SIZE];
	};
}