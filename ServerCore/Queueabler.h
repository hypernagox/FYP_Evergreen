#pragma once
#include "ServerCorePch.h"
#include "EBR.hpp"

namespace ServerCore
{
	constinit extern thread_local class Queueabler* LCurQueueableComponent;

	class Queueabler
		:public IocpComponent
	{
		friend class ThreadMgr;
		friend class TaskTimerMgr;
		friend class TickTimer;
		friend class ContentsEntity;
	public:
		Queueabler(ContentsEntity* const pOwner_);
		virtual ~Queueabler()noexcept;
	public:
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, ContentsComponent>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), ContentsComponent* const memFuncInst_, std::decay_t<Args>... args)noexcept
		{
			EnqueueAsyncTask(memFunc, static_cast<T* const>(memFuncInst_), std::move(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, ContentsComponent>
		void EnqueueAsyncPushOnly(Ret(T::* const memFunc)(Args...), ContentsComponent* const memFuncInst_, std::decay_t<Args>... args)noexcept
		{
			EnqueueAsyncTaskPushOnly(memFunc, static_cast<T* const>(memFuncInst_), std::move(args)...);
		}
	public:
		template<typename T, typename U, typename Ret, typename... Args> 
			requires (std::derived_from<T, ContentsComponent> || std::derived_from<T, IocpComponent>) && std::derived_from<T, U>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...), U* const memFuncInst_, std::decay_t<Args>... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, this,
				Task(memFunc, static_cast<T* const>(memFuncInst_), std::move(args)...));
		}
	protected:
		virtual void Dispatch(S_ptr<ContentsEntity>* const owner_entity)noexcept override final;
	private:
		template <typename Func, typename... Args>
		void EnqueueAsyncTaskPushOnly(Func&& fp, Args&&... args)noexcept;
		template <typename Func, typename... Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
		void EnqueueAsyncTask(Func&& fp, Args&&... args)noexcept;
		void Execute(S_ptr<ContentsEntity>* const owner_entity = nullptr)noexcept;
	private:
		volatile LONG m_taskCount = 0;
		MPSCQueue<Task> m_taskQueue;
	};

	template<typename Func, typename ...Args>
	inline void Queueabler::EnqueueAsyncTaskPushOnly(Func&& fp, Args && ...args) noexcept
	{
		const int32 prevCount = InterlockedIncrement(&m_taskCount);
		m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
		if (1 == prevCount)
		{
			PostIocpEvent();
		}
	}

	template<typename Func, typename ...Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
	inline void Queueabler::EnqueueAsyncTask(Func&& fp, Args && ...args) noexcept
	{
		constinit extern thread_local class Queueabler* LCurQueueableComponent;
		// TODO: 그냥 자기가 넣고 일을 한다면 레퍼런스 카운터가 올라가있는 상태여야함
		const int32 prevCount = InterlockedIncrement(&m_taskCount);
		if (1 == prevCount)
		{
			if (nullptr == LCurTaskQueue)
			{
				LCurQueueableComponent = this;
				std::invoke(std::forward<Func>(fp), std::forward<Args>(args)...);
				if (0 != InterlockedDecrement(&m_taskCount))
				{
					Execute();
				}
				else
				{
					LCurQueueableComponent = nullptr;
				}
			}
			else
			{
				m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
				PostIocpEvent();
			}
		}
		else
		{
			m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
		}
	}
}

