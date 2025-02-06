#include "ServerCorePch.h"
#include "GlobalEventQueue.h"

namespace ServerCore
{
	constinit extern thread_local uint64_t LEndTickCount;

	void GlobalEventQueue::TryGlobalEvent() noexcept
	{
		constinit extern thread_local uint64_t LEndTickCount;
		const auto end_tick = LEndTickCount;
		const BackOff bo;
		const auto arrEvent = m_arrEvent;
		do
		{
			const auto old_front = m_frontIdx;
			const auto new_front = old_front + 1;
			const auto event_ptr = arrEvent + ((new_front) & (MOD_EVENT));
			if (const auto event = event_ptr->event)
			{
				if (old_front != m_frontIdx)continue;
				if (old_front == (ULONG64)InterlockedCompareExchange64((LONG64*)&m_frontIdx, new_front, old_front))
				{
					// 문제되는 경우는, 기본적으로 한바퀴이상 돌고 여기서 컨텍스트 스위칭,
					// 또는 nullptr로 민게 반영이 안된 상태에서 IocpObject를 누가 해제하고 다른스레드가 저 위에 event_ptr로 찝어놓은 상황
					// 물론 기본적으로 한바퀴 돈채로 컨텍스트 스위칭이 나는걸 전제로함
					event_ptr->event = nullptr;
					_Compiler_barrier();
					if (const auto obj = event->GetIocpObject()) [[likely]] obj->Dispatch(event, 0);
					else [[unlikely]] PrintLogEndl(std::format("!!! {} !!!", (int)event->GetEventType<uint8_t>()));
				}
				else bo.delay();
			}
			else return;
		} while (!IocpCore::IsTimeOut(end_tick));
	}
}
