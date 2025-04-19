#include "NagiocpXPch.h"
#include "ClusterUpdateQueue.h"
#include "Field.h"
#include "Cluster.h"
#include "FieldMgr.h"

namespace NagiocpX
{
	Cluster* const GetCluster(const ClusterInfo info, Field* const field) noexcept
	{
		return field->GetCluster(info.clusterID.x, info.clusterID.y);
	}

	Field* const FieldIncRef(Field* const field) noexcept
	{
		field->IncRef(NUM_OF_THREADS);
		return field;
	}

	bool ClusterUpdateTask::Execute() noexcept
	{
		m_invoker.InvokeTask(GetCluster(m_info, m_field));
		m_field->DecRef<Field>();
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
					// 한바퀴 돌면 여기서 조금 위험할 수는 있음
					*task_ptr = nullptr;
					_Compiler_barrier();
					NagiocpX::xdelete<ClusterUpdateTask>(task);
				}
			}
			else return;
		}
	}
}