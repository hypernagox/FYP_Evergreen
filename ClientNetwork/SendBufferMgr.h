#pragma once
#include "ClientNetworkPch.h"
#include "SpinLock.hpp"

namespace NetHelper
{
	class SendBufferChunk;
	class SendBuffer;

	class SendBufferMgr
		:public Singleton<SendBufferMgr>
	{
		friend class Singleton;
		SendBufferMgr();
		~SendBufferMgr();
	public:

	public:
		static S_ptr<SendBuffer> Open(c_uint32 size_)noexcept;
	private:
		std::vector <S_ptr<SendBufferChunk>> m_chunkPool;
	private:
		void Push(S_ptr<SendBufferChunk>&& pChunk)noexcept;
		S_ptr<SendBufferChunk> Pop()noexcept;
		static void	PushGlobal(SendBufferChunk* const buffer)noexcept;
	};
}
