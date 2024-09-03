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
	public:
		Queueabler(ContentsEntity* const pOwner_);
		virtual ~Queueabler()noexcept;
	public:
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, ContentsComponent>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			EnqueueAsyncTask(memFunc, GetOwnerEntity()->GetComp<T>(), std::move(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, IocpComponent>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			EnqueueAsyncTask(memFunc, GetOwnerEntity()->GetIocpComponent<T>(), std::move(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, ContentsComponent>
		void EnqueueAsyncPushOnly(Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			EnqueueAsyncTaskPushOnly(memFunc, GetOwnerEntity()->GetComp<T>(), std::move(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, IocpComponent>
		void EnqueueAsyncPushOnly(Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			EnqueueAsyncTaskPushOnly(memFunc, GetOwnerEntity()->GetIocpComponent<T>(), std::move(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, ContentsComponent>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			const auto pOwner = GetOwnerEntity();
			const auto comp = pOwner->GetComp<T>();
			pOwner->IncRef();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, this,
				Task(memFunc, comp, std::move(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, IocpComponent>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...)noexcept, std::decay_t<Args>... args)noexcept
		{
			if (!m_taskEvent)return;
			const auto pOwner = GetOwnerEntity();
			const auto iocp_comp = pOwner->GetIocpComponent<T>();
			pOwner->IncRef();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, this,
				Task(memFunc, iocp_comp, std::move(args)...));
		}
		template<typename Ret, typename... Args>
		void EnqueueBroadcastEvent(Ret(MoveBroadcaster::* const broadCastFunc)(Args...)noexcept, MoveBroadcaster* const broadCaster, Args&&... args)noexcept
		{
			EnqueueAsyncTaskPushOnly(broadCastFunc, static_cast<MoveBroadcaster* const>(broadCaster), std::forward<Args>(args)...);
		}
		template<typename Ret, typename... Args>
		void EnqueueBroadcastEventTryExecute(Ret(MoveBroadcaster::* const broadCastFunc)(Args...)noexcept, MoveBroadcaster* const broadCaster, Args&&... args)noexcept
		{
			EnqueueAsyncTask(broadCastFunc, static_cast<MoveBroadcaster* const>(broadCaster), std::forward<Args>(args)...);
		}
	protected:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
		virtual void OnDestroy()noexcept override { EnqueueAsyncTask(&Queueabler::Destroy, this); }
	private:
		template <typename Func, typename... Args>
		void EnqueueAsyncTaskPushOnly(Func&& fp, Args&&... args)noexcept;
		template <typename Func, typename... Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
		void EnqueueAsyncTask(Func&& fp, Args&&... args)noexcept;
		void Execute()noexcept;
		void Destroy()noexcept { xdelete_sized<IocpEvent>(m_taskEvent, sizeof(IocpEvent)); m_taskEvent = nullptr; }
	private:
		std::atomic<int32> m_taskCount = 0;
		MPSCQueue<Task> m_taskQueue;
		IocpEvent* m_taskEvent = xnew<IocpEvent>(EVENT_TYPE::TASK, SharedFromThis());
	};

	template<typename Func, typename ...Args>
	inline void Queueabler::EnqueueAsyncTaskPushOnly(Func&& fp, Args && ...args) noexcept
	{
		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_seq_cst);
		m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
		if (0 == prevCount)
		{
			IncOwnerRef();
			if (m_taskEvent)
				::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, m_taskEvent);
			else
			{
				::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0
					, xnew<IocpEvent>(EVENT_TYPE::TEMPORARY, SharedFromThis()));
			}
		}
	}

	template<typename Func, typename ...Args> requires std::is_member_function_pointer_v<std::decay_t<Func>>
	inline void Queueabler::EnqueueAsyncTask(Func&& fp, Args && ...args) noexcept
	{
		constinit extern thread_local class Queueabler* LCurQueueableComponent;
		// TODO: 그냥 자기가 넣고 일을 한다면 레퍼런스 카운터가 올라가있는 상태여야함
		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_seq_cst);
		if (0 == prevCount)
		{
			if (nullptr == LCurTaskQueue)
			{
				LCurQueueableComponent = this;
				std::invoke(std::forward<Func>(fp), std::forward<Args>(args)...);
				if (1 != m_taskCount.fetch_sub(1, std::memory_order_acq_rel))
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
				IncOwnerRef();
				m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
				if (m_taskEvent)
					::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0, m_taskEvent);
				else
				{
					::PostQueuedCompletionStatus(IocpCore::GetIocpHandleGlobal(), 0, 0
						, xnew<IocpEvent>(EVENT_TYPE::TEMPORARY, SharedFromThis()));
				}
			}
		}
		else
		{
			m_taskQueue.emplace(std::forward<Func>(fp), std::forward<Args>(args)...);
		}
	}
}

