#pragma once

namespace NagiocpX
{
	class AtomicNonTemplate;
	class SendBufferChunk;
	class SendBuffer;

	/*--------------
		SendBufferMgr
	---------------*/

	class SendBufferMgr
	{
		friend class CoreGlobal;
		friend class ThreadMgr;
		SendBufferMgr()noexcept = default;
		~SendBufferMgr()noexcept = default;
	public:
		static S_ptr<SendBuffer> Open(c_uint32 size_)noexcept;
		static SendBufferChunk* const Pop()noexcept;
		static void ReturnChunk(SendBufferChunk* const chunk)noexcept;
	private:
		static void InitTLSChunkPool()noexcept;
		static void DestroyTLSChunkPool()noexcept;
	private:
		static constexpr const int32_t NUM_OF_CHUNK_BUFFER = 5;
	};

}