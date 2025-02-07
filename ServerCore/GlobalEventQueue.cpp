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
					// �����Ǵ� ����, �⺻������ �ѹ����̻� ���� ���⼭ ���ؽ�Ʈ ����Ī,
					// �Ǵ� nullptr�� �ΰ� �ݿ��� �ȵ� ���¿��� IocpObject�� ���� �����ϰ� �ٸ������尡 �� ���� event_ptr�� ������ ��Ȳ
					// ���� �⺻������ �ѹ��� ��ä�� ���ؽ�Ʈ ����Ī�� ���°� ��������
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
