#include "ServerCorePch.h"
#include "TaskQueueable.h"

namespace ServerCore
{
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;

	TaskQueueable::TaskQueueable()
		:m_taskEvent{ xnew<IocpEvent>(EVENT_TYPE::TASK, SharedFromThis()) }
	{
	}

	TaskQueueable::~TaskQueueable()noexcept
	{
	}

	void TaskQueueable::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		if (EVENT_TYPE::TEMPORARY == iocpEvent_->GetEventType())
		{
			Execute();
			xdelete_sized<IocpEvent>(iocpEvent_, sizeof(IocpEvent));
		}
		else
		{
			Execute();
		}
	}

	void TaskQueueable::EnqueueAsyncTaskPushOnly(Task&& task_) noexcept
	{
		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_seq_cst);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
			if (m_taskEvent)
				::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, m_taskEvent);
			else
			{
				::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0
					, xnew<IocpEvent>(EVENT_TYPE::TEMPORARY, SharedFromThis()));
			}
		}
	}

	void TaskQueueable::Execute()noexcept
	{
		constinit extern thread_local class TaskQueueable* LCurTaskQueue;
		constinit extern thread_local uint64 LEndTickCount;

		LCurTaskQueue = this;
		const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
		Task task;
		// TODO: 동적 월드/섹터라면 일 시작할때 카운터 올리고 끝나고 내리는걸 고려해야 할 수도 있다.
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
				if (m_taskEvent)
				{
					::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent);
				}
				else
				{
					::PostQueuedCompletionStatus(iocp_handle, 0, 0
						, xnew<IocpEvent>(EVENT_TYPE::TEMPORARY, SharedFromThis()));
				}
				break;
			}

			origin_head.store(head_temp, std::memory_order_release);
			
			if (m_taskCount.fetch_sub(taskCount, std::memory_order_seq_cst) == taskCount)
			{
				break;
			}

			if (::GetTickCount64() >= LEndTickCount)
			{
				if (m_taskEvent)
				{
					::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent);
				}
				else
				{
					::PostQueuedCompletionStatus(iocp_handle, 0, 0
						, xnew<IocpEvent>(EVENT_TYPE::TEMPORARY, SharedFromThis()));
				}
				break;
			}
		}
		LCurTaskQueue = nullptr;
	}
}