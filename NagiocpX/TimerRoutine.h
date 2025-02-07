#pragma once
#include "NagiocpXPch.h"
#include "ContentsComponent.h"

namespace NagiocpX
{
	enum class ROUTINE_RESULT :uint8_t
	{
		STOP = 0,
		STILL_RUNNIG = 1,
	};

	class TimerRoutine
		:public IocpObject
	{
		friend class TimerHandler;
	public:
		bool IsRunning()const noexcept { return m_isRunning.load(); }
		virtual ~TimerRoutine()noexcept = default;
	protected:
		virtual void StartRoutine()noexcept = 0;
		virtual NagiocpX::ROUTINE_RESULT Routine()noexcept = 0;
		virtual void ProcessRemove()noexcept = 0;
	private:
		void StartTimer()noexcept;
		virtual void Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override final;
	private:
		void SetInterval(const uint16_t interval)noexcept { m_interval = interval; }
	private:
		NagoxAtomic::Atomic<bool> m_isRunning{ true };
		uint16_t m_interval;
		IocpEvent m_timerEvent{ EVENT_TYPE::TIMER_ROUTINE };
	};

	class TimerHandler
		:public ContentsComponent
	{
		// TODO: °Ë»ö, ¹× Disable
	public:
		CONSTRUCTOR_CONTENTS_COMPONENT(TimerHandler)
	public:
		~TimerHandler()noexcept { for (const auto& timer : m_timers)timer->m_isRunning.store_relaxed(false); }
	private:
		template<typename T>
		struct TimerRAII {
			T* const timer;
			~TimerRAII()noexcept { timer->StartTimer(); }
		};
	public:
		template<typename T, typename... Args>
		[[nodiscard]] const auto AddTimer(const uint32_t interval, Args&&... args)noexcept {
			auto timer = MakeShared<T>(std::forward<Args>(args)...);
			const auto timer_ptr = timer.get();
			GetRefCountExternal(timer_ptr) = 2;
			timer_ptr->SetInterval(interval);
			m_timerLock.lock();
			for (auto& t : m_timers) {
				if (!t->IsRunning()) {
					t.swap(timer);
					m_timerLock.unlock();
					return TimerRAII<T>{timer_ptr};
				}
			}
			m_timers.emplace_back(std::move(timer));
			m_timerLock.unlock();
			return TimerRAII<T>{timer_ptr};
		}
		template<typename T, typename... Args>
		[[nodiscard]] static const auto CreateTimerWithoutHandle(const uint32_t interval, Args&&... args)noexcept {
			const auto timer_ptr = xnew<T>(std::forward<Args>(args)...);
			timer_ptr->SetInterval(interval);
			return TimerRAII<T>{timer_ptr};
		}
	private:
		XVector<S_ptr<TimerRoutine>> m_timers;
		SRWLock m_timerLock;
	};
}

