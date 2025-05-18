#pragma once
#include "NagiocpXPch.h"
#include "DeadLockDetector.h"

namespace NagiocpX
{
    class SRWLock
    {
    public:
        SRWLock(const SRWLock&) = delete;
        SRWLock(SRWLock&&)noexcept = delete;
        SRWLock& operator=(const SRWLock&) = delete;
        SRWLock& operator=(SRWLock&&)noexcept = delete;
    public:
        inline SRWLock()noexcept :SRW_lock{ SRWLOCK_INIT } {}
        inline void lock()const noexcept { AcquireSRWLockExclusive(&SRW_lock); }
        inline void unlock()const noexcept { ReleaseSRWLockExclusive(&SRW_lock); }
        inline void lock_shared()const noexcept { AcquireSRWLockShared(&SRW_lock); }
        inline void unlock_shared()const noexcept { ReleaseSRWLockShared(&SRW_lock); }
        inline const bool try_lock()const noexcept { return TryAcquireSRWLockExclusive(&SRW_lock); }
        inline const bool try_lock_shared()const noexcept { return TryAcquireSRWLockShared(&SRW_lock); }
    private:
        mutable SRWLOCK SRW_lock = SRWLOCK_INIT;

#ifdef USE_DEADLOCK_DETECTOR
	private:
		static inline constinit std::atomic_int32_t g_lock_id = 0;
	public:
		const int32_t m_lock_id = g_lock_id.fetch_add(1);
#else
		
#endif
    };

	class SRWLockGuard
	{
	public:
		SRWLockGuard(const SRWLockGuard&) = delete;
		SRWLockGuard(SRWLockGuard&&) noexcept = delete;
		SRWLockGuard& operator=(const SRWLockGuard&) = delete;
		SRWLockGuard& operator=(SRWLockGuard&&) noexcept = delete;

#ifdef USE_DEADLOCK_DETECTOR
		inline explicit SRWLockGuard(const SRWLock& srwLock_, const char* const lockName) noexcept
			: m_srwLock{ srwLock_ }, m_lockName{ lockName }
		{
			NagiocpX::DeadLockDetector::GetInst()->AcquireLock(m_srwLock.m_lock_id, m_lockName);
			m_srwLock.lock_shared();
		}
#else
		inline explicit SRWLockGuard(const SRWLock& srwLock_) noexcept
			: m_srwLock{ srwLock_ }
		{
			m_srwLock.lock_shared();
		}
#endif

		inline ~SRWLockGuard() noexcept
		{
#ifdef USE_DEADLOCK_DETECTOR
			NagiocpX::DeadLockDetector::GetInst()->ReleaseLock(m_srwLock.m_lock_id, m_lockName);
#endif
			m_srwLock.unlock_shared();
		}
	private:
		const SRWLock& m_srwLock;
#ifdef USE_DEADLOCK_DETECTOR
		const char* const m_lockName;
#endif
	};

	class SRWLockGuardEx
	{
	public:
		SRWLockGuardEx(const SRWLockGuardEx&) = delete;
		SRWLockGuardEx(SRWLockGuardEx&&) noexcept = delete;
		SRWLockGuardEx& operator=(const SRWLockGuardEx&) = delete;
		SRWLockGuardEx& operator=(SRWLockGuardEx&&) noexcept = delete;

#ifdef USE_DEADLOCK_DETECTOR
		inline explicit SRWLockGuardEx(const SRWLock& srwLock_, const char* const lockName) noexcept
			: m_srwLock{ srwLock_ }, m_lockName{ lockName }
		{
			NagiocpX::DeadLockDetector::GetInst()->AcquireLock(m_srwLock.m_lock_id, m_lockName);
			m_srwLock.lock();
		}
#else
		inline explicit SRWLockGuardEx(const SRWLock& srwLock_) noexcept
			: m_srwLock{ srwLock_ }
		{
			m_srwLock.lock();
		}
#endif

		inline ~SRWLockGuardEx() noexcept
		{
#ifdef USE_DEADLOCK_DETECTOR
			NagiocpX::DeadLockDetector::GetInst()->ReleaseLock(m_srwLock.m_lock_id, m_lockName);
#endif
			m_srwLock.unlock();
		}

	private:
		const SRWLock& m_srwLock;
#ifdef USE_DEADLOCK_DETECTOR
		const char* const m_lockName;
#endif
	};
}