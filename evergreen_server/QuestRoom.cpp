#include "pch.h"
#include "QuestRoom.h"
#include "EntityFactory.h"
#include "Regenerator.h"
#include "PartyQuestSystem.h"
#include "ClientSession.h"
#include "TaskTimerMgr.h"
#include "Navigator.h"
#include "NavigationMesh.h"
#include "PathNPC.h"
#include "PositionComponent.h"
#include "NaviAgent.h"
#include "NaviAgent_Common.h"
#include "ClusterPredicate.h"

std::atomic_int aaaa;
QuestRoom::QuestRoom() noexcept
{
	m_field_x_scale = 1024;
	m_field_y_scale = 1024;

	m_cluster_x_scale = m_field_x_scale;
	m_cluster_y_scale = m_field_y_scale;

	InitFieldGlobal();
	InitFieldTLS();
	std::cout << ++aaaa << std::endl;

}

const Vector3 mon_quest_pos[]
{
	Vector3(-95.737885F,75.60237F,0.45216048F),
	Vector3(-84.906555F,74.91477F,1.8365619F),
	Vector3(-76.985535F,75.653595F,-2.3457224F),
	Vector3(-63.27754F,74.19354F,8.34514F),
	Vector3(-51.666164F,73.28497F,18.615622F),
	Vector3(-70.57306F,74.45939F,26.04673F),
	Vector3(-58.571342F,74.35416F,31.72524F),
};

QuestRoom::~QuestRoom() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
			NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		}
		NagiocpX::DeleteJEMallocArray(clusters);
	}
	std::cout << "퀘스트룸 소멸\n";
	std::cout << --aaaa << std::endl;
	m_ownerPartrySystem->EndFlag();
}

void QuestRoom::NotifyQuestClear(NagiocpX::ContentsEntity* const entity) const noexcept
{
	entity->GetSession()->SendAsync(Create_s2c_PARTY_QUEST_CLEAR(m_ownerPartrySystem->GetCurPartyQuestID()));
}

void QuestRoom::NotifyQuestFail(NagiocpX::ContentsEntity* const entity) const noexcept
{
	// TODO: 알리기

}

void QuestRoom::InitFieldGlobal() noexcept
{
	m_fieldID = -1;
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();

	InitMutexForBenchmark(row, col);
}

void QuestRoom::InitFieldTLS() noexcept
{
	const auto row = GetNumOfClusterRow();
	const auto col = GetNumOfClusterCol();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const auto clusters = NagiocpX::CreateJEMallocArray<XVector<NagiocpX::Cluster*>>(row);
		tl_vecClusters[i] = clusters.data();
		for (uint8 r = 0; r < row; ++r)
		{
			tl_vecClusters[i][r].reserve(col);
			for (uint8 c = 0; c < col; ++c)
			{
				tl_vecClusters[i][r].emplace_back(NagiocpX::xnew<NagiocpX::Cluster>(
					NUM_OF_GROUPS,
					NagiocpX::ClusterInfo{ m_fieldID, c, r },
					this
				));
			}
		}
	}
}

void QuestRoom::DestroyFieldTLS() noexcept
{
	const auto row = (size_t)GetNumOfClusterRow();

	for (int i = 0; i < NagiocpX::NUM_OF_THREADS; ++i)
	{
		const std::span<XVector<NagiocpX::Cluster*>> clusters{ tl_vecClusters[i], row };

		for (const auto  cluster : clusters | std::views::join)
		{
			for (auto& entities : cluster->GetEntitesExceptSession())
			{
				for (auto* entity : entities.GetItemListRef())
				{
					entity->TryOnDestroy();
				}
			}
		}
	}
}

void QuestRoom::MigrationAfterBehavior(Field* const prev_field) noexcept
{
	//std::cout << "멤버카운트 " << (int)m_numOfMember << std::endl;
	std::cout << "성공\n";
}

void QuestRoom::DecMemberCount() noexcept
{
	const auto count = m_numOfMember.fetch_sub(1);
	//std::cout << (int)count << std::endl;
	if (1 == count)
	{
		std::cout << "삭제시작\n";
		DestroyFieldTLS();
		m_ownerPartrySystem->SetCurRoomInstance(nullptr);
	}
}

std::shared_ptr<PartyQuestSystem> QuestRoom::GetOwnerSystem() const noexcept
{
	return m_ownerPartrySystem;
}

void QuestRoom::CheckPartyQuestState()noexcept
{
	// TODO: 락 고려
	if (ProcessPartyQuest())
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_HARVEST;
		b.obj_type = 0;
		auto p = Vector3(-42.968254F, 75.610634F, -87.984F);
		//p = m_ownerPartrySystem->m_member[0]->GetOwnerEntity()->GetComp<PositionComponent>()->pos;
		b.x = p.x;
		b.y = p.y;
		b.z = p.z;
		const auto m = EntityFactory::CreateClearTree(b);
		m->SetDetailType(HARVEST_STATE::AVAILABLE);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		for (const auto& players : m_ownerPartrySystem->GetPartyMembers())
		{
			if (!players)continue;
			NotifyQuestClear(players.get());
		}
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		// TODO 근본적인 해결책
		//auto owner = m_ownerPartrySystem->m_member[0];
		m_isClear.store(true);
		//Mgr(TaskTimerMgr)->ReserveAsyncTask(1000,[this, owner]() {
		//	//for (const auto& players : owner->m_party_quest_system->m_member)
		//	//{
		//	//	if (!players)continue;
		//	//	//NotifyQuestClear(players->GetOwnerEntity());
		//	//}
		//	m_isClear.store(true);
		//	//m_ownerPartrySystem->GetPartyLeader()->m_cur_my_party_system.load()->MissionEnd();
		//	});
		return;
	}
	else if (IsFailPartyQuest())
	{
		for (const auto& players : m_ownerPartrySystem->GetPartyMembers())
		{
			if (!players)continue;
			NotifyQuestFail(players.get());
		}
		return;
	}
}


void FoxQuest::InitQuestField() noexcept
{
	//for (int i = 0; i < 500; ++i)
	//{
	//	EntityBuilder b;
	//	b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
	//	b.obj_type = MONSTER_TYPE_INFO::FOX;
	//	const auto m = EntityFactory::CreateMonster(b);
	//	static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
	//	const auto pos = m->GetComp<PositionComponent>()->pos;
	//	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	//	//EnterFieldNPC(m);
	//}

	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));

	m_mon_count.store_relaxed(num);

	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = MONSTER_TYPE_INFO::FOX;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();

		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
	}
}

void GoblinQuest::InitQuestField() noexcept
{
	//for (int i = 0; i < 500; ++i)
	//{
	//	EntityBuilder b;
	//	b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
	//	b.obj_type = 0;
	//	const auto m = EntityFactory::CreateRangeMonster(b);
	//	static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
	//
	//	const auto pos = m->GetComp<PositionComponent>()->pos;
	//	EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	//	//EnterFieldNPC(m);
	//}

	const auto num = (int)(sizeof(mon_quest_pos) / sizeof(mon_quest_pos[0]));

	m_mon_count.store_relaxed(num);

	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE_NPC;
		b.obj_type = 0;
		const auto m = EntityFactory::CreateRangeMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		m->GetComp<PositionComponent>()->pos = mon_quest_pos[i];
		m->GetComp<NaviAgent>()->SetPos(mon_quest_pos[i]);
		const auto pos = m->GetComp<PositionComponent>()->pos;
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
	}
}

void NPCGuardQuest::InitQuestField() noexcept
{
	EntityBuilder b;
	b.group_type = Nagox::Enum::GROUP_TYPE_MONSTER;
	b.obj_type = 0;
	const auto m = EntityFactory::CreatePathNPC(b);
	const auto m2 = m;

	const Vector3 begin = Vector3(-270.50497F, 86.48416F, -23.966377F);
	const Vector3 end = { -119.499115f,75,13.64f }; // 마을 중앙

	EnterFieldWithFloatXYNPC(begin.x + 512.f, begin.z + 512.f, m);
	//EnterFieldNPC(m);
	// TODO: 위험
	m2->GetComp<PathNPC>()->m_owner_system = GetOwnerSystem();
	m2->GetComp<PathNPC>()->InitPathNPC();

	const Vector3 points[] = {
	Vector3(-259.22272F,84.94523F,-15.314469F),
	Vector3(-242.5948F,83.53157F,-20.12327F)  ,
	Vector3(-244.0131F,83.93936F,-1.8343055F) ,
	Vector3(-225.44817F,81.969826F,-12.672854F),
	Vector3(-210.61905F,81.896484F,5.847359F) ,
	Vector3(-199.39998F,80.16998F,-11.756538F),
	Vector3(-192.54158F,80.21647F,5.781999F)  ,
	Vector3(-174.05774F,78.48746F,-9.923426F) ,
	Vector3(-158.33324F,77.9395F,5.0919523F)  ,
	};
	const auto num = sizeof(points) / sizeof(points[0]);
	for (int i = 0; i < num; ++i)
	{
		EntityBuilder b;
		b.group_type = Nagox::Enum::GROUP_TYPE::GROUP_TYPE_MONSTER;
		b.obj_type = MONSTER_TYPE_INFO::FOX;
		const auto m = EntityFactory::CreateMonster(b);
		//static_cast<Regenerator*>(m->GetDeleter())->m_targetField = SharedFromThis<NagiocpX::Field>();
		//m->GetComp<PositionComponent>()->pos = points[i];
		auto p = points[i];
		//float f[3]{ 10,10000,10 };
		//auto p2 = p;
		//dtPolyRef ref;
		//NAVIGATION->GetNavMesh(NUM_0)->GetNavMeshQuery()->findNearestPoly(&p.x, f,
		//	NAVIGATION->GetNavMesh(NUM_0)->GetNavFilter(), &ref, &p2.x
		//);

		//p.y = NAVIGATION->GetNavMesh(NUM_0)->GetNaviCell(p).CalculateHeight(p, NAVIGATION->GetNavMesh(NUM_0));
		m->GetComp<NaviAgent>()->SetPos(p);
		m->GetComp<PositionComponent>()->pos = p;
		const auto pos = p;
		//m->GetComp<NaviAgent>()->InitCrowd();
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m);
		//EnterFieldNPC(m);
	}
}

