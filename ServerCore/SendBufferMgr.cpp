#include "ServerCorePch.h"
#include "SendBufferMgr.h"
#include "SendBuffer.h"
#include "SendBufferChunk.h"

namespace ServerCore
{
	struct ChunkStack {
		struct Node {
			SendBufferChunk* const chunk;
			Node* const prev;
		};
	public:
		void push(SendBufferChunk* const chunk)noexcept {
			const auto prev_top = top;
			top = xnew<Node>(chunk, prev_top);
		}
		SendBufferChunk* const Pop()noexcept {
			if (const auto old_top = top)
			{
				top = old_top->prev;
				const auto data = old_top->chunk;
				xdelete<Node>(old_top);
				return data;
			}
			else
				return nullptr;
		}
		void Clear()noexcept {
			// TLS쓰면 간혹가다 8바이트씩 메타데이터 릭나는 버그있어서 소멸자 안만들었다.
			auto node = top;
			top = nullptr;
			while (node) {
				const auto prev = node->prev;
				VirtualFree(node->chunk, 0, MEM_RELEASE);
				xdelete<Node>(node);
				node = prev;
			}
		}
	private:
		Node* top = nullptr;
	};

	constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
	thread_local ChunkStack tl_chunkBufferPool;	// 일반벡터쓰면 TLS 소멸자 호출안되는 버그있음

	S_ptr<SendBuffer> SendBufferMgr::Open(c_uint32 size_)noexcept
	{
		constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
		extern thread_local ChunkStack tl_chunkBufferPool;

		NAGOX_ASSERT(false == LSendBufferChunk->IsOpen());

		if (size_ > LSendBufferChunk->FreeSize())
		{
			const auto new_chunk = Pop();
			GetRefCountExternal(new_chunk) = 1;
			new_chunk->Reset();
			LSendBufferChunk->DecRef<SendBufferChunk>();
			LSendBufferChunk = new_chunk;
		}

		return LSendBufferChunk->Open(size_);
	}

	SendBufferChunk* const SendBufferMgr::Pop()noexcept
	{ 
		extern thread_local ChunkStack tl_chunkBufferPool;
		if (const auto chunk = tl_chunkBufferPool.Pop())
			return chunk;
		else
			return virtual_xnew<SendBufferChunk>();
	}
	void SendBufferMgr::ReturnChunk(SendBufferChunk* const chunk) noexcept
	{
		extern thread_local ChunkStack tl_chunkBufferPool;
		tl_chunkBufferPool.push(chunk);
	}
	void SendBufferMgr::InitTLSChunkPool() noexcept
	{
		for (int i = 0; i < NUM_OF_CHUNK_BUFFER; ++i)
			tl_chunkBufferPool.push(virtual_xnew<SendBufferChunk>());
	}
	void SendBufferMgr::DestroyTLSChunkPool() noexcept
	{
		tl_chunkBufferPool.Clear();
	}
}