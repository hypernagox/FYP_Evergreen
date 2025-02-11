#pragma once
#include "NagiocpXPch.h"
#include "ThreadMgr.h"

namespace NagiocpX
{
	struct EBRNode
	{
		uint64_t remove_point;
	};

	template <typename T>
	struct EBRBox
		:public EBRNode
	{
		T box_object;
		template <typename... Args>
		EBRBox(Args&&... args)noexcept :box_object{ std::forward<Args>(args)... } {}

		static EBRNode* const GetEBRNodeAddress(void* const box_ptr) noexcept {
			return reinterpret_cast<EBRNode* const>(reinterpret_cast<char* const>(box_ptr) - offsetof(EBRBox, box_object));
		}
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
			InterlockedExchange64((LONG64*)(m_thCounter + thIdx), InterlockedIncrement64((LONG64*)&m_eCounter));
		}
		inline void End()const noexcept
		{
			const int32 thIdx = ThreadMgr::GetCurThreadIdx();
			InterlockedExchange64((LONG64*)(m_thCounter + thIdx), 0);
		}
	private:
		inline const uint64_t GetMaxEpoch()const noexcept {
			uint64_t max_epoch = std::numeric_limits<uint64_t>::min();
			auto b = m_thCounter;
			const auto e = b + ThreadMgr::NUM_OF_THREADS;
			while (e != b) {
				_Compiler_barrier();
				const auto val = (b++)->data;
				max_epoch = std::max(max_epoch, val);
			}
			return max_epoch;
		}
		inline const uint64_t GetMinEpoch()const noexcept {
			uint64_t min_epoch = std::numeric_limits<uint64_t>::max();
			auto b = m_thCounter;
			const auto e = b + ThreadMgr::NUM_OF_THREADS;
			while (e != b)
			{
				_Compiler_barrier();
				const uint64_t e = (b++)->data;
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
		inline void SetRemovePoint(uint64_t& remove_point)const noexcept { remove_point = GetMaxEpoch(); }
		inline const bool NowUsing(const uint64_t remove_point)const noexcept { return remove_point >= GetMinEpoch(); }
		constexpr EBR()noexcept = default;
	private:
		alignas(64) mutable volatile ULONG64 m_eCounter = 0;
		mutable CacheLineSeperator<volatile ULONG64> m_thCounter[ThreadMgr::NUM_OF_THREADS] = {};
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
		static const auto GetEBRPool()noexcept
		{
			constinit thread_local EBRPool* L_ebr_ptr = nullptr;
			if (nullptr == L_ebr_ptr)[[unlikely]]
				L_ebr_ptr = &GetEBRPoolRef();
			return L_ebr_ptr;
		}
		template <typename... Args>
		static T* const GetNewNode(Args&&... args)noexcept { return EBRPool<T>::GetEBRPool()->PopNode(std::forward<Args>(args)...); }
		static void RemoveNode(EBRNode* const node)noexcept { return  EBRPool<T>::GetEBRPool()->PushNode(node); }
		[[nodiscard]] static const EBR_RAII StartEBRScope()noexcept { return EBR_RAII{ *GetEBRPool() }; }
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
		void Clear()noexcept
		{
			for (int i = 0; i < ThreadMgr::NUM_OF_THREADS; ++i)
			{
				Clear(i);
			}
		}
		static auto& GetEBRPoolRef()noexcept
		{
			static EBRPool ebr_pool;
			return ebr_pool;
		}
	public:
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
		CacheLineSeperator<XQueue<T*>> m_freeList[ThreadMgr::NUM_OF_THREADS];
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
		tbb::concurrent_priority_queue <EBRNode*, EBRCompare> m_ebrPQ;
	};
}