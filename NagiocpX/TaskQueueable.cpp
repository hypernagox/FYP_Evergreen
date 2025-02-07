#include "NagiocpXPch.h"
#include "TaskQueueable.h"

namespace NagiocpX
{
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;

	TaskQueueable::TaskQueueable()
		:m_taskEvent{ EVENT_TYPE::TASK }
	{
	}

	TaskQueueable::~TaskQueueable()noexcept
	{
	}

	void TaskQueueable::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		S_ptr<IocpObject> cur_space{ iocpEvent_->PassIocpObject() };
		Execute(&cur_space);
	}

	void TaskQueueable::EnqueueAsyncTaskPushOnly(Task&& task_) noexcept
	{
		const int32 prevCount = m_taskCount.fetch_add(1);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
			m_taskEvent.SetIocpObject(S_ptr<IocpObject>{this});
			GlobalEventQueue::PushGlobalEvent(&m_taskEvent);
			//::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, m_taskEvent.GetOverlappedAddr());
		}
	}

	void TaskQueueable::Execute(S_ptr<IocpObject>* const cur_ptr)noexcept
	{
		constinit extern thread_local class TaskQueueable* LCurTaskQueue;
		constinit extern thread_local uint64 LEndTickCount;

		LCurTaskQueue = this;
		//const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
		Task task;
		auto& origin_head = m_taskQueue.head_for_single_pop();
		auto head_temp = origin_head.load(std::memory_order_seq_cst);
		for (;;)
		{
			int32_t taskCount = 0;
			while (true == m_taskQueue.try_pop_single(task,head_temp)) {
				task.ExecuteTask();
				//std::destroy_at(&task);
				++taskCount;
			}
			
			if (0 == taskCount)
			{
				m_taskEvent.SetIocpObject(cur_ptr ? std::move(*cur_ptr) : S_ptr<IocpObject>{ this });
				GlobalEventQueue::PushGlobalEvent(&m_taskEvent);
				//::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent.GetOverlappedAddr());
				break;
			}

			origin_head.store(head_temp, std::memory_order_relaxed);
			
			if (m_taskCount.fetch_sub(taskCount) == taskCount)
			{
				break;
			}

			if (::GetTickCount64() >= LEndTickCount)
			{
				m_taskEvent.SetIocpObject(cur_ptr ? std::move(*cur_ptr) : S_ptr<IocpObject>{ this });
				GlobalEventQueue::PushGlobalEvent(&m_taskEvent);
				//::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent.GetOverlappedAddr());
				break;
			}
		}
		LCurTaskQueue = nullptr;
	}
}