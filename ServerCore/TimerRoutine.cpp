#include "ServerCorePch.h"
#include "TimerRoutine.h"

namespace ServerCore
{
	void TimerRoutine::StartTimer() noexcept
	{
		m_timerEvent.SetIocpObject(S_ptr<IocpObject>{(uint64_t)this});
		StartRoutine();
		GlobalEventQueue::PushGlobalEvent(&m_timerEvent);
	}

	void TimerRoutine::Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		S_ptr<IocpObject> temp{ iocpEvent_->PassIocpObject() };
		if (!m_isRunning.load_relaxed())return;
		if (ROUTINE_RESULT::STOP == Routine())
		{
			ProcessRemove();
			m_isRunning.store_relaxed(false);
			std::atomic_thread_fence(std::memory_order_release);
		}
		else
		{
			iocpEvent_->SetIocpObject(std::move(temp));
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_interval, iocpEvent_);
		}
	}
}
