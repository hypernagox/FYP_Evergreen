#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------
	DeadLockProfiler
---------------------*/

namespace ServerCore
{
	class DeadLockProfiler
		: public Singleton<DeadLockProfiler>
	{
		friend class Singleton;
		DeadLockProfiler();
		~DeadLockProfiler();
	public:
		void PushLock(const char* name);
		void PopLock(const char* name);
		void CheckCycle();

	private:
		void Dfs(int32 index);

	private:
		std::unordered_map<const char*, int32>	_nameToId;
		std::unordered_map<int32, const char*>	_idToName;
		std::map<int32, std::set<int32>>				_lockHistory;

		Mutex _lock;

	private:
		std::vector<int32>	_discoveredOrder; // 노드가 발견된 순서를 기록하는 배열
		int32			_discoveredCount = 0; // 노드가 발견된 순서
		std::vector<bool>	_finished; // Dfs(i)가 종료 되었는지 여부
		std::vector<int32>	_parent;
	};
}
