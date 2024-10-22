#pragma once
#include "Task.h"

namespace ServerCore
{
	static constexpr const uint64_t MAX_TASK = 1024 * 1024 * 16;

	Cluster* GetCluster(const ClusterInfo info)noexcept;

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
			if (1 == m_refCount.fetch_sub(1))
			{
				ServerCore::xdelete<ClusterUpdateTask>(this);
				return true;
			}
			else
			{
				return false;
			}
		}
	
	private:
		const ClusterInfo m_info;
		const TaskInvoker m_invoker;
		std::atomic_uint8_t m_refCount = ThreadMgr::NUM_OF_THREADS;
	};

	class ClusterUpdateQueue
	{
	public:
		static void PushTask(ClusterUpdateTask* const task)noexcept {
			const auto cur_idx = m_taskCount.fetch_add(1) & (MAX_TASK - 1);
			m_arrTask[cur_idx].store(task);
		}
		static ClusterUpdateTask* const GetTask(const uint64_t idx)noexcept {
			if (idx < m_taskCount)
				return m_arrTask[idx];
			else
				return nullptr;
		}
		static void PopTask(const uint64_t idx)noexcept {
			m_arrTask[idx].store(nullptr);
		}
	public:
		static void UpdateCluster()noexcept {
			constinit thread_local uint64_t L_CurIndex = 0;
			for (;;)
			{
				const auto task = GetTask(L_CurIndex);
				if (!task)return;
				const bool bIsFinish = task->Execute(); 
				if (bIsFinish)PopTask(L_CurIndex);
				L_CurIndex = (L_CurIndex + 1) & (MAX_TASK - 1);
			}
		}
	private:
		static inline std::atomic<ClusterUpdateTask*> m_arrTask[MAX_TASK] = {};
		static inline std::atomic_uint64_t m_taskCount = 0;
	};
}

