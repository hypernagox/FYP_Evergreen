#pragma once
#include "ThreadMgr.h"
#include "NagoxAtomic.h"

namespace NagiocpX
{
	class Task;
	class TaskQueueable;
	class Queueabler;
	class IocpEvent;

	struct TimerTask
	{
		uint64 executeTime;
		mutable Task taskPtr;
		constexpr TimerTask()noexcept = default;
		constexpr ~TimerTask()noexcept = default;
		TimerTask(const TimerTask&) = delete;
		void operator=(const TimerTask&) = delete;
		constexpr TimerTask(TimerTask&& other)noexcept :executeTime{ other.executeTime }, taskPtr{ std::move(other.taskPtr) } {}
		constexpr void operator=(TimerTask&& other)noexcept
		{
			if (&other != this) [[likely]]
			{
				executeTime = other.executeTime;
				taskPtr = std::move(other.taskPtr);
			}
		}
		template <typename... Args>
		constexpr TimerTask(const uint64_t tickCount, Args&&... args)noexcept :executeTime{ tickCount }, taskPtr{ std::forward<Args>(args)... } {}
	};

	struct TimerCompare
	{
		const bool operator () (const TimerTask& a, const TimerTask& b) const noexcept { return a.executeTime > b.executeTime; }
	};

	class TaskTimerMgr
		:public Singleton<TaskTimerMgr>
	{
		friend class Singleton;
		TaskTimerMgr();
		~TaskTimerMgr();
	public:
		void ReserveAsyncTask(c_uint64 tickAfter, S_ptr<TaskQueueable>&& memfuncInstance, Task&& task)noexcept;
		void ReserveAsyncTask(c_uint64 tickAfter, Queueabler* const queueabler, Task&& task)noexcept;
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		void ReserveAsyncTask(c_uint64 tickAfter, Func&& fp, Args&&... args)noexcept
		{
			// 스트레스테스트때문에 씀, 가급적 쓰지않는다.
			m_timerTaskQueue.emplace(
				::GetTickCount64() + tickAfter, [task = Task(std::forward<Func>(fp), std::forward<Args>(args)...)]()mutable noexcept
				{
					Mgr(ThreadMgr)->EnqueueGlobalTask(std::move(task));
				});
		}
		void ReserveAsyncTask(c_uint64 tickAfter, IocpEvent* const pTimerEvent_)noexcept;
		void DistributeTask()noexcept;
	private:
		tbb::concurrent_priority_queue <TimerTask, TimerCompare, StlAllocator64<TimerTask>> m_timerTaskQueue;
		int64_t pad[7];
		volatile CHAR m_timerTaskFlag = false;
	};
}