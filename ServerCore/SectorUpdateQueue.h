#pragma once
#include "Task.h"

namespace ServerCore
{
	static constexpr const uint64_t MAX_TASK = 1024 * 1024;

	class SectorUpdateTask
	{
	public:
		bool Execute(void* sector_ptr)noexcept {
			m_invoker.ExecuteSectorTask(sector_ptr);
			if (1 == m_refCount.fetch_sub(1))
			{
				ServerCore::xdelete<SectorUpdateTask>(this);
				return true;
			}
			else
			{
				return false;
			}
		}
	private:
		std::atomic_uint8_t m_refCount = ThreadMgr::NUM_OF_THREADS;
		TaskInvoker m_invoker;
	};
	class SectorUpdateQueue
	{
	public:
		static void PushTask(SectorUpdateTask* const task)noexcept {
			const auto cur_idx = m_taskCount.fetch_add(1) & (MAX_TASK - 1);
			m_arrTask[cur_idx].store(task);
		}
		static SectorUpdateTask* const GetTask(const uint64_t idx)noexcept {
			if (idx < m_taskCount)
				return m_arrTask[idx];
			else
				return nullptr;
		}
		static void PopTask(const uint64_t idx)noexcept {
			m_arrTask[idx].store(nullptr);
		}
	public:
		static void UpdateSector()noexcept {
			for (;;)
			{
				const auto task = GetTask(0);// 스레드로칼인덱스
				const bool bIsFinish = task->Execute(nullptr); //적절한포인터
				if (bIsFinish)PopTask(0);//적절한인덱스
				// TODO: 인덱스 증가
			}
		}
	private:
		static inline std::atomic<SectorUpdateTask*> m_arrTask[MAX_TASK];
		static inline std::atomic_uint64_t m_taskCount = 0;
	};
}

