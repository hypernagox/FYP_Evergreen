#pragma once
#include "Task.h"

namespace NagiocpX
{
	Cluster* const GetCluster(const ClusterInfo info, Field* const field)noexcept;

	class ClusterUpdateTask
	{
		friend class ClusterUpdateQueue;
	public:
		template<typename... Args>
		ClusterUpdateTask(const ClusterInfo info, Field* const field, Args&&... args)noexcept
			: m_info{ info }
			, m_invoker{ std::forward<Args>(args)... }
			, m_refCount{ ThreadMgr::NUM_OF_THREADS }
			, m_field{ field }
		{}
	private:
		bool Execute()noexcept;
	private:
		volatile LONG m_refCount = ThreadMgr::NUM_OF_THREADS;
		const ClusterInfo m_info;
		Field* const m_field;
		const TaskInvoker m_invoker;
	};

	class ClusterUpdateQueue
	{
		friend class ThreadMgr;
		friend class CoreGlobal;
		static constexpr const uint64_t MAX_TASK = 1024 * 1024 * 16 * 2;
		static constexpr const uint64_t MOD_TASK = MAX_TASK - 1;
	public:
		ClusterUpdateQueue() = delete;
		~ClusterUpdateQueue()noexcept = delete;
	public:
		static inline void PushClusterTask(ClusterUpdateTask* const task)noexcept {
			const auto arrTask = m_arrTask;
			InterlockedExchangePointer((PVOID*)(arrTask + (((ULONG64)InterlockedIncrement64((LONG64*)&m_taskIdx)) & (MOD_TASK)))
				, task);
		}
		template<typename... Args>
		static inline void PushClusterTask(Args&&... args)noexcept {
			PushClusterTask(xnew<ClusterUpdateTask>(std::forward<Args>(args)...));
		}
	private:
		static void UpdateCluster()noexcept;
	private:
		static void Init() noexcept {
			static_assert(isPowerOfTwo(MAX_TASK));
			m_arrTask = (decltype(m_arrTask))VirtualAlloc(nullptr, sizeof(m_arrTask[0]) * MAX_TASK,
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			_Post_ _Notnull_ m_arrTask;
			::memset((void*)m_arrTask, 0, sizeof(m_arrTask[0]) * MAX_TASK);
		}
		static void Free() noexcept { VirtualFree((void*)m_arrTask, 0, MEM_RELEASE); }
	private:
		constinit __declspec(align(64)) static inline ClusterUpdateTask* volatile* m_arrTask = nullptr;
		constinit __declspec(align(64)) static inline volatile uint64_t m_taskIdx = -1;
	};

}

