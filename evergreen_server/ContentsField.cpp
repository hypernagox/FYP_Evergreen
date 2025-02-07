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
	//static NagiocpX::Timer t;
	//t.Update();
	//NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0)->GetCrowd()->update(t.GetDT(), 0);
	//Mgr(TaskTimerMgr)->ReserveAsyncTask(200, []() {UPDATE(); });
}

void ContentsField::InitFieldGlobal() noexcept
{
	//for (int i = 0; i < NagiocpX::ThreadMgr::NUM_OF_THREADS; ++i)
	//{
	//	m_vecClusters[i].resize(1);
	//	m_vecClusters[i][0].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{0, 0, 0}));
	//}
	m_numOfClusters = 1;
	//UPDATE();
}

void ContentsField::InitFieldTLS() noexcept
{
	const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(m_numOfClusters);
	tl_vecClusters = clusters.data();
	
	tl_vecClusters[0].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{ 0, 0, 0 }));
}

void ContentsField::DestroyFieldTLS() noexcept
{
	const std::span< XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters,m_numOfClusters };
	for (const auto cluster : clusters | std::views::join)NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
	NagiocpX::DeleteJEMallocArray(clusters);
}

ContentsField::~ContentsField()
{
	//for (int i = 0; i < NagiocpX::ThreadMgr::NUM_OF_THREADS; ++i)
	//{
	//	NagiocpX::xdelete<NagiocpX::Cluster>(
	//		m_vecClusters[i][0][0]);
	//}
}
