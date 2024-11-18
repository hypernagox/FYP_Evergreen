#pragma once
#include "Task.h"

namespace ServerCore
{
	Cluster* const GetCluster(const ClusterInfo info)noexcept;
	inline Cluster* const GetCluster(const uint8_t fieldID, const uint8_t x, const uint8_t y)noexcept {
		return GetCluster({ fieldID,x,y });
	}

	class ClusterUpdateTask
	{
	public:
		template<typename... Args>
		ClusterUpdateTask(const ClusterInfo info,Args&&... args)noexcept
			: m_info{ info }
			, m_invoker{ std::forward<Args>(args)... }
			, m_refCount{ ThreadMgr::NUM_OF_THREADS }
		{}
	public:
		bool Execute()noexcept {
			m_invoker.InvokeTask(GetCluster(m_info));
			return 0 == InterlockedDecrement(&m_refCount);
		}
	
	private:
		volatile LONG m_refCount = ThreadMgr::NUM_OF_THREADS;
		const ClusterInfo m_info;
		const TaskInvoker m_invoker;
	};

	class ClusterUpdateQueue
	{
		static constexpr const uint64_t MAX_TASK = 1024 * 1024 * 16;
	public:
		ClusterUpdateQueue() = delete;
		~ClusterUpdateQueue() = delete;
	public:
		static void PushTask(ClusterUpdateTask* const task)noexcept {
			const auto cur_idx = (ULONG64)InterlockedIncrement64((LONG64*)&m_taskCount);
			InterlockedExchangePointer((PVOID*)(m_arrTask + ((cur_idx - 1) & (MAX_TASK - 1)))
				, task);
		}
	public:
		static void UpdateCluster()noexcept {
			constinit thread_local uint64_t L_CurIndex = 0;
			for (;;)
			{
				const auto cur_idx = (L_CurIndex) & (MAX_TASK - 1);
				_Compiler_barrier();
				const auto task_ptr = m_arrTask + cur_idx;
				const auto task = *task_ptr;
				if (!task)return;
				if (task->Execute())
				{
					InterlockedExchangePointer((PVOID*)(task_ptr)
						, nullptr);
					ServerCore::xdelete<ClusterUpdateTask>(task);
				}
				++L_CurIndex;
			}
		}
	private:
		__declspec(align(8)) static inline ClusterUpdateTask* volatile m_arrTask[MAX_TASK] = { nullptr };
		__declspec(align(8)) static inline volatile uint64_t m_taskCount = 0;
	};
}

