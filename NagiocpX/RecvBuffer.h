#pragma once

/*--------------
	RecvBuffer
---------------*/

namespace NagiocpX
{
	class RecvBuffer
	{
		static constexpr const inline uint32_t RECV_BUF_SIZE = 0x10000; // 64KB
		enum { BUFFER_COUNT = RECV_BUF_SIZE / PAGE_SIZE};
		enum { BUF_SLOT_SIZE = RECV_BUF_SIZE / BUFFER_COUNT };// 4KB
	public:
		RecvBuffer()noexcept;
		~RecvBuffer()noexcept { VirtualFree(m_buffer, 0, MEM_RELEASE); }
	public:
		void Clear()noexcept;
		void ResetBufferCursor()noexcept { m_readPos = m_writePos = 0; }
		bool OnRead(c_int32 numOfBytes)noexcept
		{
			if (numOfBytes > DataSize()) [[unlikely]]
				return false;

			m_readPos += numOfBytes;

			return true;
		}
		bool OnWrite(c_int32 numOfBytes)noexcept
		{
			if (numOfBytes > FreeSize()) [[unlikely]]
				return false;

			m_writePos += numOfBytes;

			return true;
		}

		inline BYTE* const ReadPos() noexcept { return m_buffer + m_readPos; }
		inline BYTE* const WritePos()const noexcept { return const_cast<BYTE* const>(m_buffer + m_writePos); }
		inline c_int32 DataSize()const noexcept { return m_writePos - m_readPos; }
		inline c_int32 FreeSize()const noexcept { return m_capacity - m_writePos; }
	private:
		uint32_t m_readPos = 0;
		uint32_t m_writePos = 0;
		BYTE* const m_buffer = (BYTE*)VirtualAlloc(nullptr, RECV_BUF_SIZE,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		static constexpr inline const uint32_t m_capacity = RECV_BUF_SIZE;
	};
}

