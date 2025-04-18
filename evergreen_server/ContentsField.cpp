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
	m_numOfClusters = 1;
	m_fieldID = 0;
	//UPDATE();
}

void ContentsField::InitFieldTLS() noexcept
{
	const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(m_numOfClusters);
	tl_vecClusters[NagiocpX::GetCurThreadIdx()] = clusters.data();
	
	{
		tl_vecClusters[NagiocpX::GetCurThreadIdx()][0].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(NUM_OF_GROUPS, NagiocpX::ClusterInfo{ m_fieldID, 0, 0 }, this));
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

ContentsField::~ContentsField()
{
	//for (int i = 0; i < NagiocpX::ThreadMgr::NUM_OF_THREADS; ++i)
	//{
	//	NagiocpX::xdelete<NagiocpX::Cluster>(
	//		m_vecClusters[i][0][0]);
	//}
}
