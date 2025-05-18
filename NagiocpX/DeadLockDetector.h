#pragma once
#include "Singleton.hpp"
#include <unordered_map>
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
		std::string GetLockUsedPosition(const int32_t id)noexcept {
			std::string temp{ '(' };
			for (const auto str_ptr : m_idToName[id]) {
				temp += str_ptr + std::string{ ", " };
			}
			return temp + ')';
		}
	private:
		std::mutex m_lock;
		std::map<const char*, int32_t>	m_nameToId;
		std::map<int32, std::vector<const char*>> m_idToName;
		std::map<int32_t, std::set<int32_t>> m_lockHistory;
	

		std::vector<int32_t> m_discoveredOrder;
		int32_t m_discoveredCount = 0;
		std::vector<bool> m_finished;
		std::vector<int32_t> m_parent;
	};
}
