#include "ServerCorePch.h"
#include "TaskTimerMgr.h"
#include "TaskQueueable.h"
#include "ThreadMgr.h"
#include "IocpEvent.h"
#include "Queueabler.h"

namespace ServerCore
{
	TaskTimerMgr::TaskTimerMgr()
	{
		for (int i = 0; i < 1024 * 1024 * 2; ++i)
		{
			m_timerTaskQueue.emplace(TimerTask{});
		}
		m_timerTaskQueue.clear();
	}

	TaskTimerMgr::~TaskTimerMgr()
	{
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, S_ptr<TaskQueueable>&& memfuncInstance, Task&& task)noexcept
	{
		m_timerTaskQueue.emplace(
			::GetTickCount64() + tickAfter,
			[memfuncInstance = std::move(memfuncInstance), task = std::move(task)]()mutable noexcept
			{
				memfuncInstance->EnqueueAsyncTaskPushOnly(std::move(task));
			}
		);
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, Queueabler* const queueabler, Task&& task)noexcept
	{
		queueabler->SetIncRefEntity();
		m_timerTaskQueue.emplace(
			::GetTickCount64() + tickAfter,
			[queueabler, task = std::move(task)]()mutable noexcept
			{
				queueabler->EnqueueAsyncTaskPushOnly(queueabler->PassEntity(), std::move(task));
			}
		);
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, IocpEvent* const pTimerEvent_) noexcept
	{
		m_timerTaskQueue.emplace(
			::GetTickCount64() + tickAfter,
			[pTimerEvent_]()noexcept
			{
				GlobalEventQueue::PushGlobalEventRelaxed(pTimerEvent_);
				//::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, pTimerEvent_->GetOverlappedAddr());
			}
		);
	}

	void TaskTimerMgr::DistributeTask()noexcept
	{
		if (!m_timerTaskFlag && !InterlockedExchange8(&m_timerTaskFlag, true))
		{
			TimerTask task;
			while (m_timerTaskQueue.try_pop(task))
			{
				if (::GetTickCount64() < task.executeTime)
				{
					m_timerTaskQueue.emplace(std::move(task));
					break;
				}
				else
				{
					task.taskPtr.ExecuteTask();
					//std::destroy_at<TimerTask>(&task);
				}
			}

			InterlockedExchange8(&m_timerTaskFlag, false);
		}
	}
}