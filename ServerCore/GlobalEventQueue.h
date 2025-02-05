#pragma once
#include "ServerCorePch.h"
#include "BackOff.h"

namespace ServerCore
{
	class IocpEvent;
	class IocpObject;

	class GlobalEventQueue
	{
		friend class TaskTimerMgr;
		friend class ThreadMgr;
		friend class CoreGlobal;
		static constexpr const uint64_t MAX_EVENT = (1024 * 1024 * 16 * 2);
		static constexpr const uint64_t MOD_EVENT = MAX_EVENT - 1;
	public:
		GlobalEventQueue() = delete;
		~GlobalEventQueue()noexcept = delete;
	public:
		static inline void PushGlobalEvent(IocpEvent* const event)noexcept {
			const auto arrEvent = m_arrEvent;
			InterlockedExchangePointer((PVOID*)(arrEvent + (((ULONG64)InterlockedIncrement64((LONG64*)&m_rearIdx)) & (MOD_EVENT))), event);
		}
	private:
		static inline void PushGlobalEventRelaxed(IocpEvent* const event)noexcept {
			const auto arrEvent = m_arrEvent;
			arrEvent[(((ULONG64)InterlockedIncrement64((LONG64*)&m_rearIdx)) & (MOD_EVENT))].event = event;
		}
		static void TryGlobalEvent()noexcept;
	private:
		static void Init()noexcept {
			m_arrEvent = (IocpEvent64*)_aligned_malloc(sizeof(m_arrEvent[0]) * MAX_EVENT, 64);
			_Post_ _Notnull_ m_arrEvent;
			::memset(m_arrEvent, 0, sizeof(m_arrEvent[0]) * MAX_EVENT);
		}
		static void Free()noexcept { _aligned_free(m_arrEvent); }
	private:
		struct alignas(64) IocpEvent64 { alignas(64) IocpEvent* volatile event; };
		constinit __declspec(align(64)) static inline IocpEvent64* m_arrEvent = nullptr;
		constinit __declspec(align(64)) static inline volatile uint64_t m_frontIdx = -1;
		constinit __declspec(align(64)) static inline volatile uint64_t m_rearIdx = -1;
	};
}




