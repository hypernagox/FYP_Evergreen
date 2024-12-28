#pragma once
#include "Task.h"
#include "TaskTimerMgr.h"
#include "ThreadMgr.h"
#include "IocpObject.h"
#include "IocpEvent.h"
#include "NagoxAtomic.h"

namespace ServerCore
{
	class TaskQueueable
		:public IocpObject
	{
		friend class ThreadMgr;
		friend class TaskTimerMgr;
	public:
		TaskQueueable();
		virtual ~TaskQueueable()noexcept;
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, std::move(ptr), std::forward<Args>(args)...);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, ptr, std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, static_cast<T* const>(this), std::forward<Args>(args)...);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), S_ptr<U>&& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, std::move(ptr), std::forward<Args>(args)...);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, ptr, std::forward<Args>(args)...);
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...> && IsNotMemFunc<Func>
		void EnqueueAsync(Func&& fp, Args&&... args)noexcept
		{
			// TODO: 쌩 람다 넘어오면 어떻게 할 지 다시 고민하기
			EnqueueAsyncTask(std::forward<Func>(fp), std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			EnqueueAsyncTask(memFunc, static_cast<T* const>(this), std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, S_ptr<T>{ this },
				Task(memFunc, static_cast<T* const>(this), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, S_ptr<T>{ this },
				Task(memFunc, static_cast<T* const>(this), std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Func&& fp, Args&&... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, S_ptr<TaskQueueable>{this}, Task(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...);
		}
	protected:
		inline const bool IsValid()const noexcept { return m_bValid.load(); }
	private:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
		void EnqueueAsyncTaskPushOnly(Task&& task_)noexcept;
		template <typename Func, typename... Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
		void EnqueueAsyncTask(Func&& fp, Args&&... args)noexcept;
		void Execute(S_ptr<IocpObject>* const cur_ptr = nullptr)noexcept;
		
	private:
		NagoxAtomic::Atomic<int32> m_taskCount{ 0 };
		MPSCQueue<Task> m_taskQueue;
		IocpEvent m_taskEvent;

		NagoxAtomic::Atomic<bool> m_bValid{ true };
	};

	template<typename Func, typename ...Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
	inline void TaskQueueable::EnqueueAsyncTask(Func&& fp, Args && ...args) noexcept
	{
		constinit extern thread_local class TaskQueueable* LCurTaskQueue;
		const int32 prevCount = m_taskCount.fetch_add(1);
		if (0 == prevCount)
		{
			if (nullptr == LCurTaskQueue)
			{
				LCurTaskQueue = this;
				std::invoke(std::forward<Func>(fp), std::forward<Args>(args)...);
				if (1 != m_taskCount.fetch_sub(1))
				{
					Execute();
				}
				else
				{
					LCurTaskQueue = nullptr;
				}
			}
			else
			{
				m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
				m_taskEvent.SetIocpObject(S_ptr<IocpObject>{this});
				::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, m_taskEvent.GetOverlappedAddr());
			}
		}
		else
		{
			m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
		}
	}
}
