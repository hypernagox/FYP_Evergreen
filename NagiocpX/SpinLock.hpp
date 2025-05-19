#pragma once
#include "NagiocpXPch.h"
#include "DeadLockDetector.h"

namespace NagiocpX
{
    class SpinLock
    {
    public:
        inline SpinLock() noexcept = default;
        inline ~SpinLock() noexcept = default;
        inline void lock()const noexcept {
            for (;;) {
                while (lockFlag);
                if (!InterlockedCompareExchange((LONG*)&lockFlag, TRUE, FALSE))return;
            }
        }
        inline void unlock()const noexcept {
            InterlockedExchange((LONG*)&lockFlag, FALSE);
        }
        inline const bool try_lock()const noexcept {
            return !InterlockedCompareExchange((LONG*)&lockFlag, TRUE, FALSE);
        }
    private:
        alignas(4) mutable volatile BOOL lockFlag = FALSE;

#ifdef USE_DEADLOCK_DETECTOR
    public:
        const int32_t m_lock_id = NagiocpX::DeadLockDetector::GenerateLockID();
#else

#endif
    };

    class SpinLockGuard
    {
    public:
        SpinLockGuard(const SpinLockGuard&) = delete;
        SpinLockGuard(SpinLockGuard&&) noexcept = delete;
        SpinLockGuard& operator=(const SpinLockGuard&) = delete;
        SpinLockGuard& operator=(SpinLockGuard&&) noexcept = delete;

#ifdef USE_DEADLOCK_DETECTOR
        inline explicit SpinLockGuard(const SpinLock& spinLock_, const char* const lockName) noexcept
            : m_spinLock{ spinLock_ }, m_lockName{ lockName }
        {
            NagiocpX::DeadLockDetector::GetInst()->AcquireLock(m_spinLock.m_lock_id, m_lockName);
            m_spinLock.lock();
        }
#else
        inline explicit SpinLockGuard(const SpinLock& spinLock_) noexcept
            : m_spinLock{ spinLock_ }
        {
            m_spinLock.lock();
        }
#endif

        inline ~SpinLockGuard() noexcept
        {
#ifdef USE_DEADLOCK_DETECTOR
            NagiocpX::DeadLockDetector::GetInst()->ReleaseLock(m_spinLock.m_lock_id, m_lockName);
#endif
            m_spinLock.unlock();
        }

    private:
        const SpinLock& m_spinLock;
#ifdef USE_DEADLOCK_DETECTOR
        const char* const m_lockName;
#endif
    };
}