#include "ServerCorePch.h"
#include "Queueabler.h"

namespace ServerCore
{
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class Queueabler* LCurQueueableComponent;

	Queueabler::Queueabler(ContentsEntity* const pOwner_)
		: IocpComponent{ pOwner_ }
		, m_taskEvent{ xnew<IocpEvent>(EVENT_TYPE::TASK, SharedFromThis()) }
	{
	}

	Queueabler::~Queueabler()noexcept
	{
		//std::cout << "È®ÀÎ" << std::endl;
	}

	void Queueabler::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
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
		DecOwnerRef();
	}

	void Queueabler::Execute()noexcept
	{
		constinit extern thread_local uint64 LEndTickCount;
		constinit extern thread_local class Queueabler* LCurQueueableComponent;

		LCurQueueableComponent = this;
		const HANDLE iocp_handle = IocpCore::GetIocpHandleGlobal();
		Task task;
		bool flag = false;
		auto& origin_head = m_taskQueue.head_for_single_pop();
		auto head_temp = origin_head.load(std::memory_order_seq_cst);
		for (;;)
		{
			int32_t taskCount = 0;
			while (true == m_taskQueue.try_pop_single(task, head_temp)) {
				task.ExecuteTask();
				//std::destroy_at(&task);
				++taskCount;
			}
			
			if (0 == taskCount)
			{
				if (flag)
				{
					if (m_taskEvent)
					{
						IncOwnerRef();
						::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent);
						break;
					}
					else
					{
						std::this_thread::yield();
						continue;
					}
				}
				else
				{
					flag = true;
					std::this_thread::yield();
					continue;
				}
			}

			origin_head.store(head_temp, std::memory_order_release);
			
			if (m_taskCount.fetch_sub(taskCount, std::memory_order_release) == taskCount)
			{
				break;
			}

			if (::GetTickCount64() >= LEndTickCount)
			{
				if (m_taskEvent)
				{
					IncOwnerRef();
					::PostQueuedCompletionStatus(iocp_handle, 0, 0, m_taskEvent);
					break;
				}
				else
					continue;
			}
		}
		LCurQueueableComponent = nullptr;
	}
}
