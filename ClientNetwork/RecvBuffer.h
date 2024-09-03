#pragma once

namespace NetHelper
{
	class RecvBuffer
	{
		enum { BUFFER_COUNT = 10 };
	public:
		RecvBuffer(c_int32 bufferSize_)noexcept;
		~RecvBuffer();
	public:
		enum RECV_BUFFER_SIZE
		{
			BUFFER_SIZE = 0x10000, // 64KB
		};

		void Clear()noexcept;
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

		BYTE* const ReadPos() noexcept { return m_buffer + m_readPos; }
		BYTE* const WritePos() noexcept { return m_buffer + m_writePos; }
		c_int32 DataSize()const noexcept { return m_writePos - m_readPos; }
		c_int32 FreeSize()const noexcept { return m_capacity - m_writePos; }
	private:
		const int32 m_bufferSize;
		const int32 m_capacity;
		int32 m_readPos = 0;
		int32 m_writePos = 0;
		BYTE m_buffer[BUFFER_SIZE * BUFFER_COUNT];
	};
}
