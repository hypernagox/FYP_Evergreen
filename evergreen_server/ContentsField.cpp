#include "pch.h"
#include "ContentsField.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "Timer.h"
#include "Field.h"
#include "Cluster.h"

void UPDATE()
{
	static ServerCore::Timer t;
	t.Update();
	NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->update(t.GetDT(), 0);
	Mgr(TaskTimerMgr)->ReserveAsyncTask(200, []() {UPDATE(); });
}

void ContentsField::InitField() noexcept
{
	for (int i = 0; i < ServerCore::ThreadMgr::NUM_OF_THREADS; ++i)
	{
		m_vecClusters[i].resize(1);
		m_vecClusters[i][0].emplace_back(ServerCore::xnew<ServerCore::Cluster>(NUM_OF_GROUPS, ServerCore::ClusterInfo{0, 0, 0}));
	}
	UPDATE();
}

ContentsField::~ContentsField()
{
	for (int i = 0; i < ServerCore::ThreadMgr::NUM_OF_THREADS; ++i)
	{
		ServerCore::xdelete<ServerCore::Cluster>(
			m_vecClusters[i][0][0]);
	}
}
