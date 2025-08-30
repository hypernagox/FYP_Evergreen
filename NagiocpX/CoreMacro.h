#pragma once

#define OUT

#define DECLARE_SINGLETON(ClassName) \
    friend class Singleton;          \
    ClassName()noexcept = default;   \
    ~ClassName()noexcept = default;

#define MEMBER_SIZE_SUM(Type, lastMember) (offsetof(Type, lastMember) + sizeof(((Type*)0)->lastMember))

#define PAGE_SIZE (0x1000)

//#define USE_DEADLOCK_DETECTOR

#ifdef USE_DEADLOCK_DETECTOR

#define SHARED_LOCK(valName) \
    NagiocpX::SRWLockGuard     srw_lock_guard_##valName { valName, __FUNCTION__ }

#define EXCLUSIVE_LOCK(valName) \
    NagiocpX::SRWLockGuardEx   srw_lock_guard_ex_##valName { valName, __FUNCTION__ }

#define SPIN_LOCK(valName) \
    NagiocpX::SpinLockGuard    spin_lock_guard_##valName { valName, __FUNCTION__ }

#else

#define SHARED_LOCK(name) \
    NagiocpX::SRWLockGuard     srw_lock_guard_##name { name }

#define EXCLUSIVE_LOCK(name) \
    NagiocpX::SRWLockGuardEx   srw_lock_guard_ex_##name { name }

#define SPIN_LOCK(name) \
    NagiocpX::SpinLockGuard    spin_lock_guard_##name { name }

#endif




// #define TRACK_FUNC_LOG
// #define TRACK_LOG
//#define USE_NAGOX_ASSERT
#define USE_PRINT_ERROR

#ifdef TRACK_FUNC_LOG
#define CREATE_FUNC_LOG(msg) const auto FUNC_LOG = Mgr(Logger)->CreateFuncLog(msg)
#else
#define CREATE_FUNC_LOG(msg)
#endif

#ifdef TRACK_LOG
#define LOG_MSG(msg) (Mgr(Logger)->EnqueueLogMsg(msg))
#else
#define LOG_MSG(msg)
#endif

#ifdef USE_NAGOX_ASSERT

#define NAGOX_ASSERT(condition) \
    do { \
        if (!(condition)) [[unlikely]] { \
			NagiocpX::LogStackTrace(); \
            *(int*)nullptr = 0; \
        } \
    } while (0)

#else

#define NAGOX_ASSERT(condition)  do { (void)(condition); } while (0)
#endif

#ifdef USE_NAGOX_ASSERT

#define NAGOX_ASSERT_LOG(condition, log) \
    do { \
        if (!(condition)) [[unlikely]] { \
            std::cerr << "Activate NagOx Assertion: " << log << '\n'; \
            LogStackTrace(); \
            *(int*)nullptr = 0; \
        } \
    } while (0)

#else

#define NAGOX_ASSERT_LOG(condition, log) do { (void)(condition); (void)(log); } while (0)

#endif

#ifdef USE_PRINT_ERROR

#define PRINT_ERROR(condition, log, err_code) \
        if (!(condition)) [[unlikely]] \
            PrintError(log,err_code); 
#else

#define PRINT_ERROR(condition, log, err_code)

#endif

#pragma warning(disable: 4554)