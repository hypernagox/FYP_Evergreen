#pragma once

/*--------------
	RecvBuffer
---------------*/

namespace ServerCore
{
	class alignas(64) RecvBuffer
	{
		static constexpr const inline uint32_t DESIRE_BUF_SIZE = 0x10000; // 64KB
		enum { BUFFER_COUNT = 16 };
		enum RECV_BUFFER_SIZE
		{
			BUFFER_SIZE = DESIRE_BUF_SIZE / BUFFER_COUNT // 4KB
		};
	public:
		RecvBuffer()noexcept;
		~RecvBuffer()noexcept = default;
	public:
		static constexpr const inline uint32_t ACTUAL_BUF_SIZE = (BUFFER_SIZE * BUFFER_COUNT) - 4;
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
		uint16_t m_readPos = 0;
		uint16_t m_writePos = 0;
		static constexpr inline const uint16_t m_capacity = ACTUAL_BUF_SIZE;
		BYTE m_buffer[ACTUAL_BUF_SIZE];
	};
}

