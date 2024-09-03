#pragma once
#include "ServerCorePch.h"
#include "ThreadMgr.h"

namespace ServerCore
{
	struct EBRNode
	{
		uint64_t remove_point;
	};

	template <typename T>
	struct alignas(64) CacheLineSeperator
	{
		T data;
	};

	struct EBRCompare
	{
		const bool operator () (const EBRNode* const a, const EBRNode* const b) const noexcept { return a->remove_point > b->remove_point; }
	};

	class EBR
	{
	public:
		inline void Start()const noexcept
		{
			const int32 thIdx =ThreadMgr::GetCurThreadIdx();
			m_thCounter[thIdx].data.store(
				m_eCounter.fetch_add(1, std::memory_order_seq_cst),
				std::memory_order_seq_cst
			);
		}
		inline void End()const noexcept
		{
			const int32 thIdx = ThreadMgr::GetCurThreadIdx();
			m_thCounter[thIdx].data.store(0, std::memory_order_seq_cst);
		}
		inline void SetRemovePoint(std::atomic<uint64_t>& remove_point)const noexcept { remove_point.store(GetMaxEpoch(), std::memory_order_release); }
		inline void SetRemovePoint(uint64_t& remove_point)const noexcept { remove_point = GetMaxEpoch(); std::atomic_thread_fence(std::memory_order_release); }
		inline const bool NowUsing(const uint64_t remove_point)const noexcept { return remove_point >= GetMinEpoch(); }
	private:
		inline const uint64_t GetMaxEpoch()const noexcept {
			uint64_t max_epoch = std::numeric_limits<uint64_t>::min();
			for (int i = 0; i < ThreadMgr::NUM_OF_THREADS; ++i)
				max_epoch = std::max(max_epoch, m_thCounter[i].data.load(std::memory_order_acquire));
			return max_epoch;
		}
		inline const uint64_t GetMinEpoch()const noexcept {
			uint64_t min_epoch = std::numeric_limits<uint64_t>::max();
			for (int i = 0; i < ThreadMgr::NUM_OF_THREADS; ++i)
			{
				const uint64_t e = m_thCounter[i].data.load(std::memory_order_acquire);
				if (0 != e)min_epoch = std::min(min_epoch, e);
			}
			return min_epoch;
		}
	protected:
		template<typename T, typename... Args>
		static T* const AllocNode(Args&&... args)noexcept {
			if constexpr (alignof(T) >= 64)
				return aligned_xnew<T>(std::forward<Args>(args)...);
			else
				return xnew<T>(std::forward<Args>(args)...);
		}
	private:
		mutable std::atomic<uint64_t> m_eCounter = 0;
		mutable CacheLineSeperator<std::atomic<uint64_t>> m_thCounter[ThreadMgr::NUM_OF_THREADS] = {};
	};

	
	class EBR_RAII
	{
	public:
		inline explicit EBR_RAII(const EBR& ebr_)noexcept :ebr{ ebr_ } { ebr_.Start(); }
		inline ~EBR_RAII()noexcept { ebr.End(); }
	private:
		const EBR& ebr;
	};

	template <typename T> requires std::derived_from<T, EBRNode>
	class EBRPool
		:public EBR
	{
	public:
		template <typename... Args>
		static T* const GetNewNode(Args&&... args)noexcept { return EBRPool<T>::GetEBRPool().PopNode(std::forward<Args>(args)...); }
		static void RemoveNode(EBRNode* const node)noexcept { return  EBRPool<T>::GetEBRPool().PushNode(node); }
		static const EBR_RAII StartEBRScope()noexcept { return EBR_RAII{ GetEBRPool() }; }
	public:
		EBRPool(const EBRPool&) = delete;
		EBRPool(EBRPool&&)noexcept = delete;
	private:
		EBRPool()noexcept = default;
		~EBRPool()noexcept { Clear(); }
		void Clear(const int idx)noexcept
		{
			auto& q = m_freeList[idx].data;
			while (false == q.empty())
			{
				if constexpr (alignof(T) >= 64)
					aligned_xdelete_sized<T>(q.front(), sizeof(T), alignof(T));
				else
					xdelete_sized<T>(q.front(), sizeof(T));
				q.pop();
			}
		}
		static auto& GetEBRPool()noexcept
		{
			static EBRPool ebr_pool;
			return ebr_pool;
		}
		void Clear()noexcept
		{
			for (int i = 0; i < ThreadMgr::NUM_OF_THREADS; ++i)
			{
				Clear(i);
			}
		}
		template <typename... Args>
		T* const PopNode(Args&&... args)noexcept
		{
			const int32 thIdx = ThreadMgr::GetCurThreadIdx();
			auto& freeList = m_freeList[thIdx].data;
			if (freeList.empty())return AllocNode<T>(std::forward<Args>(args)...);
			T* const node = freeList.front();
			if (NowUsing(node->remove_point))return AllocNode<T>(std::forward<Args>(args)...);
			freeList.pop();
			std::destroy_at<T>(node);
			return std::construct_at<T>(node, std::forward<Args>(args)...);
		}
		void PushNode(EBRNode* const node)noexcept
		{
			const int32 thIdx = ThreadMgr::GetCurThreadIdx();
			SetRemovePoint(node->remove_point);
			m_freeList[thIdx].data.emplace(static_cast<T* const>(node));
		}
	private:
		CacheLineSeperator<Queue<T*>> m_freeList[ThreadMgr::NUM_OF_THREADS];
	};

	template <typename T> requires std::derived_from<T, EBRNode>
	class EBRQueue
		:public EBR
	{
	public:
		template <typename... Args>
		static T* const GetNewNode(Args&&... args)noexcept { return EBRQueue<T>::GetEBRQueue().PopNode(std::forward<Args>(args)...); }
		static void RemoveNode(EBRNode* const node)noexcept { return  EBRQueue<T>::GetEBRQueue().PushNode(node); }
		static const EBR_RAII StartEBRScope()noexcept { return EBR_RAII{ EBRQueue<T>::GetEBRQueue() }; }
	public:
		EBRQueue(const EBRQueue&) = delete;
		EBRQueue(EBRQueue&&)noexcept = delete;
	private:
		EBRQueue(const uint32_t default_pq_size)noexcept {
			for (uint32_t i = 0; i < default_pq_size; ++i)m_ebrPQ.emplace(nullptr);
			m_ebrPQ.clear();
		}
		~EBRQueue()noexcept { Clear(); }
		void Clear(const int idx)noexcept
		{
			EBRNode* temp;
			while (false == m_ebrPQ.try_pop(temp)) { 
				if constexpr (alignof(T) >= 64)
					aligned_xdelete_sized<T>(temp, sizeof(T), alignof(T));
				else
					xdelete_sized<T>(temp,sizeof(T));
			}
		}
		static auto& GetEBRQueue()noexcept
		{
			static EBRQueue ebr_queue{ 1024 };
			return ebr_queue;
		}
		template <typename... Args>
		T* const PopNode(Args&&... args)noexcept
		{
			EBRNode* node;
			if (true == m_ebrPQ.try_pop(node))
			{
				if (true == NowUsing(node->remove_point))
				{
					m_ebrPQ.emplace(node);
					return AllocNode<T>(std::forward<Args>(args)...);
				}
				else
				{
					std::destroy_at<T>(node);
					return std::construct_at<T>(static_cast<T*const>(node), std::forward<Args>(args)...);
				}
			}
			else
			{
				return AllocNode<T>(std::forward<Args>(args)...);
			}
		}
		void PushNode(EBRNode* const node)noexcept
		{
			SetRemovePoint(node->remove_point);
			m_ebrPQ.emplace(node);
		}
	private:
		tbb::concurrent_priority_queue <EBRNode*, EBRCompare, StlAllocator64<EBRNode*>> m_ebrPQ;
	};
}