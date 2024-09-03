#pragma once

/*--------------
	SendBuffer
---------------*/

namespace ServerCore
{
	class SendBufferChunk;

	class SendBuffer
		:public RefCountable
	{
	public:
		SendBuffer(const S_ptr<SendBufferChunk>& owner, BYTE* const buffer, c_uint32 allocSize_)noexcept
			: m_pOwnerChunk{ owner }
			, m_buffer{ buffer }
			, m_allocSize{ allocSize_ }
			, m_writeSize{ 0 }
		{}
		~SendBuffer()noexcept = default;
	public:
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