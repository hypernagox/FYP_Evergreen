#include "ServerCorePch.h"
#include "Queueabler.h"

namespace ServerCore
{
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class Queueabler* LCurQueueableComponent;

	Queueabler::Queueabler(ContentsEntity* const pOwner_)
		: IocpComponent{ pOwner_ ,IOCP_COMPONENT::Queueabler }
	{
	}

	Queueabler::~Queueabler()noexcept
	{
		//std::cout << "È®ÀÎ" << std::endl;
	}

	void Queueabler::Dispatch(S_ptr<ContentsEntity>* const owner_entity) noexcept
	{
		Execute(owner_entity);
	}

	void Queueabler::Execute(S_ptr<ContentsEntity>* const owner_entity)noexcept
	{
		constinit extern thread_local uint64 LEndTickCount;
		constinit extern thread_local class Queueabler* LCurQueueableComponent;

		LCurQueueableComponent = this;
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
					PostIocpEvent(owner_entity);
					break;
				}
				else
				{
					flag = true;
					std::this_thread::yield();
					continue;
				}
			}

			origin_head.store(head_temp, std::memory_order_relaxed);
			
			if (0 == InterlockedAdd(&m_taskCount, -taskCount))
			{
				break;
			}

			if (::GetTickCount64() >= LEndTickCount)
			{
				PostIocpEvent(owner_entity);
				break;
			}
		}
		LCurQueueableComponent = nullptr;
	}
}
