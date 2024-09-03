#pragma once

namespace NetHelper
{
	class SendBufferChunk;

	class SendBuffer
	{
	public:
		SendBuffer(S_ptr<SendBufferChunk> owner, BYTE* const buffer, c_uint32 allocSize_)noexcept;
		~SendBuffer()noexcept;

		BYTE* const Buffer()noexcept { return m_buffer; }
		c_uint32 WriteSize()const noexcept { return m_writeSize; }
		void Close(c_uint32 writeSize_)noexcept;

	private:
		BYTE* const m_buffer;
		c_uint32 m_allocSize;
		uint32 m_writeSize = 0;
		const S_ptr<SendBufferChunk> m_pOwnerChunk;
	};
}
