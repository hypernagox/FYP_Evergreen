#include "pch.h"
#include "ContentsField.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "Timer.h"
#include "Field.h"
#include "Cluster.h"
#include "QuestRoom.h"
#include "ClientSession.h"

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
	m_fieldID = 0;

	m_field_x_scale = 1024;
	m_field_y_scale = 1024;

	m_cluster_x_scale = DISTANCE_FILTER;
	m_cluster_y_scale = DISTANCE_FILTER;

	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();
	InitMutexForBenchmark(row, col);
	//UPDATE();
}

void ContentsField::InitFieldTLS() noexcept
{
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();
	
	const auto th_idx = NagiocpX::GetCurThreadIdx();
	const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(row);
	tl_vecClusters[th_idx] = clusters.data();

	for (int i = 0; i < row; ++i)
	{
		tl_vecClusters[th_idx][i].reserve(col);
		for (int j = 0; j < col; ++j)
		{
			tl_vecClusters[th_idx][i].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{m_fieldID, (uint8)j, (uint8)i}, this));
		}
	}
}

void ContentsField::MigrationAfterBehavior(Field* const prev_field) noexcept
{
	if (-1 == prev_field->GetFieldID())
	{
		const auto q = static_cast<QuestRoom*>(prev_field);
		q->DecMemberCount();
	}
	std::cout << "이주 성공\n";
}

ContentsField::ContentsField()
{
	m_fieldID = 0;

	m_field_x_scale = 1024;
	m_field_y_scale = 1024;

	m_cluster_x_scale = (int)DISTANCE_FILTER;
	m_cluster_y_scale = (int)DISTANCE_FILTER;

	//const auto row = GetNumOfClusterRow();
	//const auto col = GetNumOfClusterCol();
	//
	//
	//
	//for (int t = 0; t < NagiocpX::NUM_OF_THREADS; ++t)
	//{
	//	const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(1);
	//	tl_vecClusters[t] = clusters.data();
	//	for (int i = 0; i < row; ++i)
	//	{
	//		for (int j = 0; j < col; ++j)
	//		{
	//			tl_vecClusters[t][i].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{ m_fieldID, (uint8)j, (uint8)i }, this));
	//		}
	//	}
	//}
}

ContentsField::~ContentsField()
{
	//for (int i = 0; i < NagiocpX::ThreadMgr::NUM_OF_THREADS; ++i)
	//{
	//	NagiocpX::xdelete<NagiocpX::Cluster>(
	//		m_vecClusters[i][0][0]);
	//}
}
