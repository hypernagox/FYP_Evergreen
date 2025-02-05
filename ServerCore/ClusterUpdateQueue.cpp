#include "ServerCorePch.h"
#include "ClusterUpdateQueue.h"
#include "Field.h"
#include "Cluster.h"
#include "FieldMgr.h"

namespace ServerCore
{
	Cluster* const GetCluster(const ClusterInfo info) noexcept
	{
		return Mgr(FieldMgr)->GetCluster(info);
	}

	bool ClusterUpdateTask::Execute() noexcept
	{
		m_invoker.InvokeTask(GetCluster(m_info));
		return 0 == InterlockedDecrement(&m_refCount);
	}

	void ClusterUpdateQueue::UpdateCluster() noexcept
	{
		constinit thread_local uint64_t L_CurIndex = 0;
		const auto arrTask = m_arrTask;
		for (;;) [[likely]]
		{
			const auto task_ptr = arrTask + ((L_CurIndex) & (MOD_TASK));
			if (const auto task = *task_ptr)
			{
				++L_CurIndex;
				if (task->Execute())
				{
					// �ѹ��� ���� ���⼭ ���� ������ ���� ����
					*task_ptr = nullptr;
					_Compiler_barrier();
					ServerCore::xdelete<ClusterUpdateTask>(task);
				}
			}
			else return;
		}
	}
}