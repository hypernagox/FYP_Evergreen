#pragma once

/*--------------
	SendBuffer
---------------*/

namespace NagiocpX
{
	class SendBufferChunk;

	class SendBuffer final
		:public RefCountable
	{
	public:
		constexpr inline SendBuffer(SendBufferChunk* const owner, BYTE* const buffer, c_uint32 allocSize_)noexcept
			: m_pOwnerChunk{ owner }
			, m_buffer{ buffer }
			, m_allocSize{ allocSize_ }
			, m_writeSize{ 0 }
		{}
		constexpr inline ~SendBuffer()noexcept { m_pOwnerChunk->DecRef<SendBufferChunk>(); }
	public:
		BYTE* const Buffer()noexcept { return m_buffer; }
		c_uint32 WriteSize()const noexcept { return m_writeSize; }
		void Close(c_uint32 writeSize_)noexcept;
	private:
		BYTE* const m_buffer;
		c_uint32 m_allocSize;
		uint32 m_writeSize = 0;
		SendBufferChunk* const m_pOwnerChunk;
	};
}