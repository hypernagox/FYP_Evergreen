#pragma once

/*--------------
	SendBufferChunk
---------------*/

namespace NagiocpX
{
	class SendBuffer;

	class SendBufferChunk final
		:public RefCountable
	{
		static constexpr const inline uint32_t SEND_BUFFER_CHUNK_SIZE = 0x200000; // 2MB
		friend class SendBufferMgr;
	public:
		SendBufferChunk()noexcept = default;
		~SendBufferChunk()noexcept { VirtualFree(m_buffer, 0, MEM_RELEASE); }
		void Close(c_uint32 writeSize_)noexcept
		{
			NAGOX_ASSERT(true == m_bOpen);
			m_bOpen = false;
			m_usedSize += writeSize_;
		}
	private:
		S_ptr<SendBuffer> Open(c_uint32 allocSize)noexcept;
		void Reset()noexcept { m_curPage = m_usedSize = m_bOpen = false; }
		const bool IsOpen()const noexcept { return m_bOpen; }
		BYTE* const Buffer()noexcept { return m_buffer + m_usedSize; }
		c_uint32 FreeSize()const noexcept { return SEND_BUFFER_CHUNK_SIZE - m_usedSize; }
	private:
		int64_t pad[7];
		bool m_bOpen = false;
		uint32_t m_curPage = 0;
		uint32_t m_usedSize = 0;
		BYTE* const m_buffer = (BYTE*)VirtualAlloc(nullptr, SEND_BUFFER_CHUNK_SIZE,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	};
}
