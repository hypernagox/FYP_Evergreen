#pragma once

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

#define USE_NET_NAGOX_ASSERT

#ifdef USE_NET_NAGOX_ASSERT

#define NET_NAGOX_ASSERT(condition) \
    do { \
        if (!(condition)) [[unlikely]] { \
            std::cerr << "Activate NagOx Assertion !" << '\n'; \
			NetHelper::LogStackTrace(); \
            *(int*)nullptr = 0; \
        } \
    } while (0)

#else

#define NET_NAGOX_ASSERT(condition)

#endif

#ifdef USE_NET_NAGOX_ASSERT

#define NET_NAGOX_ASSERT_LOG(condition, log) \
    do { \
        if (!(condition)) [[unlikely]] { \
            std::cerr << "Activate NagOx Assertion: " << log << '\n'; \
            LogStackTrace(); \
            *(int*)nullptr = 0; \
        } \
    } while (0)

#else

#define NET_NAGOX_ASSERT_LOG(condition, log)

#endif

template<typename T>
using U_ptr = std::unique_ptr<T>;

template<typename T>
using S_ptr = std::shared_ptr<T>;

template <typename T>
using W_ptr = std::weak_ptr<T>;