#pragma once
#include "ServerCorePch.h"

namespace ServerCore
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
        inline const bool try_lock()const noexcept { return FALSE != TryAcquireSRWLockExclusive(&SRW_lock); }
        inline const bool try_lock_shared()const noexcept { return FALSE != TryAcquireSRWLockShared(&SRW_lock); }
    private:
        mutable SRWLOCK SRW_lock = SRWLOCK_INIT;
    };

    class SRWLockGuard
    {
    public:
        inline explicit SRWLockGuard(SRWLock& srwLock_)noexcept :m_srwLock{ srwLock_ } { m_srwLock.lock_shared(); }
        inline ~SRWLockGuard()noexcept { m_srwLock.unlock_shared(); }
    private:
        const SRWLock& m_srwLock;
    };

    class SRWLockGuardEx
    {
    public:
        inline explicit SRWLockGuardEx(SRWLock& srwLock_)noexcept :m_srwLock{ srwLock_ } { m_srwLock.lock(); }
        inline ~SRWLockGuardEx()noexcept { m_srwLock.unlock(); }
    private:
        const SRWLock& m_srwLock;
    };
}