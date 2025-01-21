#include "ServerCorePch.h"
#include "SendBufferMgr.h"
#include "SendBuffer.h"
#include "SendBufferChunk.h"

namespace ServerCore
{
	constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
	

	//template<typename T>
	//class SendBufferAllocator
	//{
	//public:
	//	using value_type = T;
	//
	//	constexpr SendBufferAllocator()noexcept {}
	//
	//	template<typename Other>
	//	SendBufferAllocator(const SendBufferAllocator<Other>&) { }
	//
	//	constexpr static T* const allocate(const size_t size)noexcept
	//	{
	//		return static_cast<T* const>(SendBufferMgrHelper::GetSendBufferMgrPool()->allocate());
	//	}
	//
	//	constexpr static void deallocate(T* const ptr, const size_t count)noexcept
	//	{
	//		SendBufferMgrHelper::GetSendBufferMgrPool()->deallocate(ptr);
	//	}
	//};

	S_ptr<SendBuffer> SendBufferMgr::Open(c_uint32 size_)noexcept
	{
		constinit extern thread_local class SendBufferChunk* LSendBufferChunk;

		NAGOX_ASSERT(false == LSendBufferChunk->IsOpen());

		// 다쓰면 교체
		if (size_ > LSendBufferChunk->FreeSize())
		{
			LSendBufferChunk->DecRef<SendBufferChunk>();
			LSendBufferChunk = Pop();
			// LSendBufferChunk->Reset();
		}

		return LSendBufferChunk->Open(size_);
	}

	SendBufferChunk* const SendBufferMgr::Pop()noexcept { return aligned_xnew<SendBufferChunk>(); }
}