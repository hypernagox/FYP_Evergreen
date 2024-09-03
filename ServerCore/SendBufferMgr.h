#pragma once

namespace ServerCore
{
	class AtomicNonTemplate;
	class SendBufferChunk;
	class SendBuffer;

	/*--------------
		SendBufferMgr
	---------------*/

	//namespace SendBufferMgrHelper {
	//	static inline AtomicNonTemplate* const GetSendBufferMgrPool()noexcept;
	//};

	class SendBufferMgr
	{
		//friend static inline AtomicNonTemplate* const SendBufferMgrHelper::GetSendBufferMgrPool()noexcept;
		SendBufferMgr()noexcept = default;
		~SendBufferMgr()noexcept = default;
	public:
		static S_ptr<SendBuffer> Open(c_uint32 size_)noexcept;
		static S_ptr<SendBufferChunk> Pop()noexcept;
	private:
		//std::byte m_pSendBufferPool[ThreadMgr::NUM_OF_THREADS][sizeof(AtomicNonTemplate)];
		//AtomicNonTemplate m_pSendBufferPool[ThreadMgr::NUM_OF_THREADS];
	};

	//namespace SendBufferMgrHelper {
	//	static inline AtomicNonTemplate* const GetSendBufferMgrPool()noexcept { 
	//		thread_local AtomicNonTemplate* const th_send_pool = reinterpret_cast<AtomicNonTemplate* const>(Mgr(SendBufferMgr)->m_pSendBufferPool[GetCurThreadIdx() % NUM_OF_THREADS]);
	//		return th_send_pool;
	//	}
	//};
}