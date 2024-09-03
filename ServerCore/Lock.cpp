#include "ServerCorePch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

namespace ServerCore
{
	void Lock::WriteLock(const char* name)
	{
#if _DEBUG
		Mgr(DeadLockProfiler)->PushLock(name);
#endif

		// 동일한 쓰레드가 소유하고 있다면 무조건 성공.
		const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
		if (LThreadId == lockThreadId)
		{
			_writeCount++;
			return;
		}

		// 아무도 소유 및 공유하고 있지 않을 때, 경합해서 소유권을 얻는다.
		const int64 beginTick = ::GetTickCount64();
		const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
		uint32 expected = EMPTY_FLAG;
		while (true)
		{
			for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
			{
				if (_lockFlag.compare_exchange_strong(OUT expected, desired))
				{
					_writeCount++;
					return;
				}
				expected = EMPTY_FLAG;
			}

			if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
				CRASH("LOCK_TIMEOUT");

			std::this_thread::yield();
		}
	}

	void Lock::WriteUnlock(const char* name)
	{
#if _DEBUG
		Mgr(DeadLockProfiler)->PopLock(name);
#endif

		// ReadLock 다 풀기 전에는 WriteUnlock 불가능.
		if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
			CRASH("INVALID_UNLOCK_ORDER");

		const int32 lockCount = --_writeCount;
		if (lockCount == 0)
			_lockFlag.store(EMPTY_FLAG);
	}

	void Lock::ReadLock(const char* name)
	{
#if _DEBUG
		Mgr(DeadLockProfiler)->PushLock(name);
#endif

		// 동일한 쓰레드가 소유하고 있다면 무조건 성공.
		const uint32 initFlagValue = _lockFlag.load();

		if ((initFlagValue & WRITE_THREAD_MASK) >> 16 == LThreadId)
		{
			_lockFlag.fetch_add(1);
			return;
		}

		// 아무도 소유하고 있지 않을 때 경합해서 공유 카운트를 올린다.
		const int64 beginTick = ::GetTickCount64();
		uint32 expected = initFlagValue & READ_COUNT_MASK;
		while (true)
		{
			for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
			{
				if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
					return;
				expected &= READ_COUNT_MASK;
			}

			if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
				CRASH("LOCK_TIMEOUT");

			std::this_thread::yield();
		}
	}

	void Lock::ReadUnlock(const char* name)
	{
#if _DEBUG
		Mgr(DeadLockProfiler)->PopLock(name);
#endif

		if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
			CRASH("MULTIPLE_UNLOCK");
	}
}