#pragma once

#define OUT

#define DECLARE_SINGLETON(ClassName) \
    friend class Singleton;          \
    ClassName()noexcept = default;   \
    ~ClassName()noexcept = default;

#define MEMBER_SIZE_SUM(Type, lastMember) (offsetof(Type, lastMember) + sizeof(((Type*)0)->lastMember))

#define PAGE_SIZE (0x1000)

/*---------------
	  Lock
---------------*/

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/*---------------
	  Crash
---------------*/

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)			\
{									\
	if (!(expr)) [[unlikely]]		\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}


// #define TRACK_FUNC_LOG
// #define TRACK_LOG
#define USE_NAGOX_ASSERT
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

#define NAGOX_ASSERT(condition)

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

#define NAGOX_ASSERT_LOG(condition, log)

#endif

#ifdef USE_PRINT_ERROR

#define PRINT_ERROR(condition, log, err_code) \
        if (!(condition)) [[unlikely]] \
            PrintError(log,err_code); 
#else

#define PRINT_ERROR(condition, log, err_code)

#endif

#pragma warning(disable: 4554)