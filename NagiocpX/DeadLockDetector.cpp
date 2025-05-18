#include "NagiocpXPch.h"
#include "DeadLockDetector.h"

namespace NagiocpX
{
	thread_local XVector<int32_t> LLockStack = {};

	DeadLockDetector::DeadLockDetector()
	{
		extern thread_local XVector<int32_t> LLockStack;
	}

	DeadLockDetector::~DeadLockDetector()
	{
		extern thread_local XVector<int32_t> LLockStack;

		printf("========== DEADLOCK DETECTOR SUMMARY ==========\n");

		for (const auto& [fromId, toSet] : m_lockHistory)
		{
			const auto fromName = GetLockUsedPosition(fromId);
			for (const int32_t toId : toSet)
			{
				const auto toName = GetLockUsedPosition(toId);
				printf("%s    ->    %s\n", fromName.c_str(), toName.c_str());
			}
		}

		printf("===============================================\n");
	}

	void DeadLockDetector::AcquireLock(const int32_t lock_id, const char* const name)noexcept
	{
		extern thread_local XVector<int32_t> LLockStack;
		const bool flag = (LLockStack.end() !=
			std::ranges::find(LLockStack, lock_id));
		{
			LockGuard guard{ m_lock };

			const auto it = m_nameToId.find(name);

			if (it == m_nameToId.end())
			{
				m_nameToId[name] = lock_id;
				m_idToName[lock_id].emplace_back(name);
			}
			
			if (flag)
			{
				printf("===== CURRENT LOCK HISTORY =====\n");

				for (const auto& [fromId, toSet] : m_lockHistory)
				{
					const auto fromName = GetLockUsedPosition(fromId);
					for (const int32_t toId : toSet)
					{
						const auto toName = GetLockUsedPosition(toId);
						printf("%s    ->    %s\n", fromName.c_str(), toName.c_str());
					}
				}

				printf("\n\n===== CURRENT LOCK STACK (THIS THREAD) =====\n");
				for (int i = 0; i < (int)LLockStack.size(); ++i)
				{
					const auto log = GetLockUsedPosition(LLockStack[i]);
					printf("[%d]: %s\n", i, log.c_str());
				}
				

				NAGOX_ASSERT_LOG(0, "RE_ENTRANT_LOCK_DETECTED (detailed)\n\n");
			}

			if (!LLockStack.empty())
			{
				if (m_lockHistory[LLockStack.back()].emplace(lock_id).second)
				{
					CheckCycle();
				}
			}
		}

		LLockStack.emplace_back(lock_id);
	}

	void DeadLockDetector::ReleaseLock(const int32_t lock_id, const char* const name)noexcept
	{
		extern thread_local XVector<int32_t> LLockStack;

		if (LLockStack.empty())
			NAGOX_ASSERT_LOG(0, "MULTIPLE_UNLOCK");

		if (lock_id != LLockStack.back())
			NAGOX_ASSERT_LOG(0, "UNLOCK_ORDER_MISTAKE");

		LLockStack.pop_back();
	}

	void DeadLockDetector::CheckCycle()noexcept
	{
		extern thread_local XVector<int32_t> LLockStack;

		const int32_t lockCount = std::max(m_idToName.rbegin()->first, (int)m_idToName.size() + 1) + 1;
		m_discoveredCount = 0;

		for (int32_t i = 0; i < lockCount; ++i)
		{
			m_discoveredOrder.emplace_back(-1);
			m_finished.emplace_back(false);
			m_parent.emplace_back(-1);
		}

		for (int32_t lockId = 0; lockId < lockCount; ++lockId) 
		{
			DFS(lockId);
		}

		m_discoveredOrder.clear();
		m_finished.clear();
		m_parent.clear();
	}

	void DeadLockDetector::DFS(const int32_t here)noexcept
	{
		extern thread_local XVector<int32_t> LLockStack;

		if (-1 != m_discoveredOrder[here])
			return;

		m_discoveredOrder[here] = m_discoveredCount++;

		const auto findIt = m_lockHistory.find(here);

		if (m_lockHistory.end() == findIt)
		{
			m_finished[here] = true;
			return;
		}

		const auto& nextSet = findIt->second;

		for (const auto there : nextSet)
		{
			if (-1 == m_discoveredOrder[there])
			{
				m_parent[there] = here;
				DFS(there);
				continue;
			}

			if (m_discoveredOrder[here] < m_discoveredOrder[there])
				continue;

			if (false == m_finished[there])
			{
				const auto here_id = GetLockUsedPosition(here);
				const auto there_id = GetLockUsedPosition(there);

				printf("%s    ->    %s\n", here_id.c_str(), there_id.c_str());

				int32_t now = here;
				for(;;)
				{
					const auto parent_ids = GetLockUsedPosition(m_parent[now]);
					const auto now_ids = GetLockUsedPosition(now);

					printf("%s    ->    %s\n", parent_ids.c_str(), now_ids.c_str());
					now = m_parent[now];
					if (now == there)break;
				}

				NAGOX_ASSERT_LOG(0, "DEADLOCK_DETECTED");
			}
		}

		m_finished[here] = true;
	}
}