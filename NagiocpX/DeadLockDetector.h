#pragma once
#include "Singleton.hpp"
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

namespace NagiocpX
{
	class DeadLockDetector
		: public Singleton<DeadLockDetector>
	{
		friend class Singleton;
		DeadLockDetector();
		~DeadLockDetector();
	public:
		void AcquireLock(const int32_t lock_id, const char* const name)noexcept;
		void ReleaseLock(const int32_t lock_id, const char* const name)noexcept;
	private:
		void CheckCycle()noexcept;
		void DFS(const int32_t index)noexcept;
		std::string GetLockUsedPosition(const int32_t id)noexcept;
#ifdef USE_DEADLOCK_DETECTOR
	public:
		static inline const auto GenerateLockID()noexcept { return g_lock_id.fetch_add(1); }
	private:
		static inline constinit std::atomic_int32_t g_lock_id = 0;
#else
#endif
	private:
		std::mutex m_lock;
		std::map<std::string, int32_t>	m_nameToId;
		std::map<int32_t, std::vector<std::string>> m_idToName;
		std::map<int32_t, std::set<int32_t>> m_lockHistory;
	
		int32_t m_discoveredCount = 0;
		std::unordered_map<int32_t, int32_t> m_discoveredOrder;
		std::unordered_set<int32_t> m_finished;
		std::unordered_map<int32_t, int32_t> m_parent;
	};
}
