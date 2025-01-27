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
			// TLS���� ��Ȥ���� 8����Ʈ�� ��Ÿ������ ������ �����־ �Ҹ��� �ȸ������.
			auto node = top;
			top = nullptr;
			while (node) {
				const auto prev = node->prev;
				Memory::AlignedFree(node->chunk, alignof(SendBufferChunk));
				xdelete<Node>(node);
				node = prev;
			}
		}
	private:
		Node* top = nullptr;
	};

	constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
	thread_local ChunkStack tl_chunkBufferPool;	// �Ϲݺ��;��� TLS �Ҹ��� ȣ��ȵǴ� ��������

	S_ptr<SendBuffer> SendBufferMgr::Open(c_uint32 size_)noexcept
	{
		constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
		extern thread_local ChunkStack tl_chunkBufferPool;

		NAGOX_ASSERT(false == LSendBufferChunk->IsOpen());

		// �پ��� ��ü
		if (size_ > LSendBufferChunk->FreeSize())
		{
			const auto new_chunk = Pop();
			new_chunk->Reset();
			LSendBufferChunk->DecRef<SendBufferChunk>();
			LSendBufferChunk = new_chunk;
		}

		return LSendBufferChunk->Open(size_);
	}

	SendBufferChunk* const SendBufferMgr::Pop()noexcept { 
		extern thread_local ChunkStack tl_chunkBufferPool;
		if (const auto chunk = tl_chunkBufferPool.Pop())
			return chunk;
		else
			return aligned_xnew<SendBufferChunk>();
	}
	void SendBufferMgr::ReturnChunk(SendBufferChunk* const chunk) noexcept
	{
		extern thread_local ChunkStack tl_chunkBufferPool;
		GetRefCountExternal(chunk) = 1;
		tl_chunkBufferPool.push(chunk);
	}
	void SendBufferMgr::InitTLSChunkPool() noexcept
	{
		for (int i = 0; i < NUM_OF_CHUNK_BUFFER; ++i)
			tl_chunkBufferPool.push(aligned_xnew<SendBufferChunk>());
	}
	void SendBufferMgr::DestroyTLSChunkPool() noexcept
	{
		tl_chunkBufferPool.Clear();
	}
}