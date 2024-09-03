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
            while (lockFlag.test_and_set(std::memory_order_acquire)) {
            }
        }
        inline void unlock()const noexcept {
            lockFlag.clear(std::memory_order_release);
        }
        inline const bool try_lock()const noexcept {
            return !lockFlag.test_and_set(std::memory_order_acquire);
        }
    private:
        mutable std::atomic_flag lockFlag = ATOMIC_FLAG_INIT;
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