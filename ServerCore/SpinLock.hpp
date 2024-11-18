#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
    class SpinLock
    {
    public:
        inline SpinLock() noexcept = default;
        inline ~SpinLock() noexcept = default;
        inline void lock()const noexcept {
            for (;;) {
                while (TRUE == lockFlag);
                if (FALSE ==
                    InterlockedCompareExchange((LONG*)&lockFlag, TRUE, FALSE))
                    return;
            }
        }
        inline void unlock()const noexcept {
            InterlockedExchange((LONG*)&lockFlag, FALSE);
        }
        inline const bool try_lock()const noexcept {
            return FALSE ==
                InterlockedCompareExchange((LONG*)&lockFlag, TRUE, FALSE);
        }
    private:
        alignas(4) mutable volatile BOOL lockFlag = FALSE;
    };

    class SpinLockGuard
    {
    public:
        inline explicit SpinLockGuard(SpinLock& spinLock_)noexcept :m_spinLock{ spinLock_ } { m_spinLock.lock(); }
        inline ~SpinLockGuard()noexcept { m_spinLock.unlock(); }
    private:
        const SpinLock& m_spinLock;
    };
}