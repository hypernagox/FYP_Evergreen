#pragma once
#include "ClientNetworkPch.h"

namespace NetHelper
{
    class SpinLock
    {
    private:
        std::atomic_flag lockFlag = ATOMIC_FLAG_INIT;
    public:
        SpinLock() noexcept = default;
        ~SpinLock() noexcept = default;
        void lock() noexcept {
            while (lockFlag.test_and_set(std::memory_order_acquire)) {
            }
        }
        void unlock() noexcept {
            lockFlag.clear(std::memory_order_release);
        }
        const bool try_lock() noexcept {
            return !lockFlag.test_and_set(std::memory_order_acquire);
        }
    };
}