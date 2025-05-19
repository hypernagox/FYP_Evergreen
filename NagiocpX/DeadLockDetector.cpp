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
			std::lock_guard<std::mutex> lock{ m_lock };

			const auto it = m_idToName.find(lock_id);

			if (m_idToName.end() == it)
			{
				auto lock_name = name + std::to_string(lock_id);
				m_nameToId.try_emplace(lock_name, lock_id);
				m_idToName[lock_id].emplace_back(std::move(lock_name));
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

		m_discoveredCount = 0;

		for (const auto lockId: m_idToName | std::ranges::views::keys)
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

		const auto iter = m_discoveredOrder.find(here);

		if (m_discoveredOrder.end() != iter)return;

		const auto hereOrder = m_discoveredCount++;

		m_discoveredOrder.emplace_hint(iter, here, hereOrder);
		
		const auto findIt = m_lockHistory.find(here);

		if (m_lockHistory.end() == findIt)
		{
			m_finished.emplace(here);
			return;
		}

		const auto& nextSet = findIt->second;

		for (const auto there : nextSet)
		{
			const auto thereIt = m_discoveredOrder.find(there);
			
			if (m_discoveredOrder.end() == thereIt)
			{
				m_parent.try_emplace(there, here);
				DFS(there);
				continue;
			}
			else
			{
				if (hereOrder < thereIt->second)
					continue;

				if (false == m_finished.contains(there))
				{
					const auto here_id = GetLockUsedPosition(here);
					const auto there_id = GetLockUsedPosition(there);

					printf("%s    ->    %s\n", here_id.c_str(), there_id.c_str());

					int32_t now = here;
					for (;;)
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
		}

		m_finished.emplace(here);
	}
	std::string DeadLockDetector::GetLockUsedPosition(const int32_t id) noexcept
	{
		std::string temp{ '(' };
		for (const auto& str : m_idToName[id]) {
			temp += str + std::string{ ", " };
		}
		return temp + ')';
	}
}